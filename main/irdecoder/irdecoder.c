// irdecoder.c

#include "irdecoder.h"
#include "dht11_task.h"
#include "speaker_driver.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lcd_task.h"
#include "rom/ets_sys.h"
#include <string.h>

static const char* TAG = "IR_DRIVER";

static uint64_t last_time = 0;

static volatile uint8_t active_idx = 0;
static volatile uint8_t decode_len = 0;

static volatile uint32_t ir_buffer_A[IR_TIMES_SIZE];
static volatile uint32_t ir_buffer_B[IR_TIMES_SIZE];

static volatile uint32_t* a_active_buffer = ir_buffer_A;
static volatile uint32_t* a_decode_buffer = ir_buffer_B;

static SemaphoreHandle_t xSignaler    = NULL;
static TimerHandle_t ir_timeout_timer = NULL;

gptimer_handle_t GPtimer;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint64_t curr_time, pulse_length;
    gptimer_get_raw_count(GPtimer, &curr_time);

    if (last_time == 0) {
        last_time = curr_time;
        return;
    }

    pulse_length = curr_time - last_time;

    BaseType_t higher_priority_task = pdFALSE;
    xTimerResetFromISR(ir_timeout_timer, &higher_priority_task);

    last_time = curr_time;

    if (pulse_length > 15000) {
        if (active_idx > 0) {
            volatile uint32_t* tmp = a_active_buffer;

            a_active_buffer = a_decode_buffer;
            a_decode_buffer = tmp;
            decode_len      = active_idx;

            xSemaphoreGiveFromISR(xSignaler, &higher_priority_task);
        }
        active_idx = 0;
    } else {
        if (active_idx < IR_TIMES_SIZE) {
            a_active_buffer[active_idx++] = pulse_length;
        } else {
            last_time  = 0;
            active_idx = 0;
        }
    }
}

static void ir_timeout_callback(TimerHandle_t xTimer) {
    volatile uint32_t* tmp = a_active_buffer;

    a_active_buffer = a_decode_buffer;
    a_decode_buffer = tmp;

    decode_len = active_idx;
    active_idx = 0;
    last_time  = 0;

    BaseType_t higher_priority_task = pdFALSE;
    xSemaphoreGiveFromISR(xSignaler, &higher_priority_task);
}

static void decoder_task_init() {
    gpio_reset_pin(IR_PIN);
    gpio_set_direction(IR_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(IR_PIN, GPIO_INTR_ANYEDGE);

    gptimer_config_t timer_config = {
        .clk_src       = GPTIMER_CLK_SRC_DEFAULT,
        .direction     = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000};

    xSignaler = xSemaphoreCreateBinary();
    if (xSignaler == NULL) {
        ESP_LOGE(TAG, "FAILED TO CREATE SEMAPHORE: UNSTABLE BEHAVIOR EXPECTED");
        return;
    }

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &GPtimer));

    esp_err_t isr_service_result = gpio_install_isr_service(0);
    if (isr_service_result != ESP_OK && isr_service_result != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "FAILED TO INSTALL ISR SERVICE: %s", esp_err_to_name(isr_service_result));
        return;
    }
    ESP_ERROR_CHECK(gpio_isr_handler_add(IR_PIN, gpio_isr_handler, NULL));

    ir_timeout_timer = xTimerCreate(
        "IRTimeout",
        pdMS_TO_TICKS(TIMEOUT_US / 1000),
        pdFALSE,
        NULL,
        ir_timeout_callback);

    if (ir_timeout_timer == NULL) {
        ESP_LOGE(TAG, "FAILED TO CREATE SOFTWARE TIMER");
    }

    gptimer_enable(GPtimer);
    gptimer_start(GPtimer);
}

static void ir_decode(ir_result_t* result) {
    result->type = IR_FRAME_TYPE_INVALID;

    result->address = 0;
    result->command = 0;

    if (decode_len > 50) {
        if (!IR_MATCH(a_decode_buffer[0], NEC_START_PULSE) || !IR_MATCH(a_decode_buffer[1], NEC_START_SPACE)) {
            ESP_LOGE(TAG, "IR SIGNAL DOESN'T FOLLOW NEC START PROTOCOL");
            return;
        }

        uint32_t decoded_data = 0;
        for (int i = 0; i < 32; i++) {
            uint32_t pulse = a_decode_buffer[i * 2 + 2];
            uint32_t space = a_decode_buffer[i * 2 + 3];

            if (!IR_MATCH(pulse, NEC_BIT_PULSE)) {
                ESP_LOGE(TAG, "IR BIT NOT A VALID BIT PULSE");
                return;
            }

            decoded_data <<= 1;
            if (IR_MATCH(space, NEC_ONE_SPACE)) {
                decoded_data |= 1;
            } else if (IR_MATCH(space, NEC_ZERO_SPACE)) {
                decoded_data |= 0;
            } else {
                ESP_LOGE(TAG, "IR BIT NOT A VALID BIT SPACE");
                return;
            }
        }

        uint8_t addr     = (decoded_data >> 24) & 0xFF;
        uint8_t inv_addr = (decoded_data >> 16) & 0xFF;
        uint8_t cmd      = (decoded_data >> 8) & 0xFF;
        uint8_t inv_cmd  = (decoded_data) & 0xFF;

        if ((addr ^ inv_addr) != 0xFF || (cmd ^ inv_cmd) != 0xFF) {
            ESP_LOGE(TAG, "CHECKSUM FAILED");
            return;
        }

        result->type = IR_FRAME_TYPE_DATA;

        result->address = addr;
        result->command = cmd;

        return;
    }

    else if (decode_len > 0 && decode_len < 10) {
        result->type = IR_FRAME_TYPE_REPEAT;
        return;
    }
}

static void decode_key_value(ir_result_t* ir_data) {
    switch (ir_data->command) {
    case BUTTON_0:
        ir_data->button = BUTTON_0;
        break;
    case BUTTON_1:
        ir_data->button = BUTTON_1;
        break;
    case BUTTON_2:
        ir_data->button = BUTTON_2;
        break;
    case BUTTON_3:
        ir_data->button = BUTTON_3;
        break;
    case BUTTON_4:
        ir_data->button = BUTTON_4;
        break;
    case BUTTON_5:
        ir_data->button = BUTTON_5;
        break;
    case BUTTON_6:
        ir_data->button = BUTTON_6;
        break;
    case BUTTON_7:
        ir_data->button = BUTTON_7;
        break;
    case BUTTON_8:
        ir_data->button = BUTTON_8;
        break;
    case BUTTON_9:
        ir_data->button = BUTTON_9;
        break;
    case BUTTON_PLUS:
        ir_data->button = BUTTON_PLUS;
        break;
    case BUTTON_MINUS:
        ir_data->button = BUTTON_MINUS;
        break;
    case BUTTON_EQ:
        ir_data->button = BUTTON_EQ;
        break;
    case BUTTON_U_SD:
        ir_data->button = BUTTON_U_SD;
        break;
    case BUTTON_CYCLE:
        ir_data->button = BUTTON_CYCLE;
        break;
    case BUTTON_PLAY_PAUSE:
        ir_data->button = BUTTON_PLAY_PAUSE;
        break;
    case BUTTON_BACKWARD:
        ir_data->button = BUTTON_BACKWARD;
        break;
    case BUTTON_FORWARD:
        ir_data->button = BUTTON_FORWARD;
        break;
    case BUTTON_POWER:
        ir_data->button = BUTTON_POWER;
        break;
    case BUTTON_MUTE:
        ir_data->button = BUTTON_MUTE;
        break;
    case BUTTON_MODE:
        ir_data->button = BUTTON_MODE;
        break;
    default:
        ir_data->button = BUTTON_UNKNOWN_OR_ERROR;
        break;
    }
}

static const char* get_button_name(button_press_t button) {
    switch (button) {
    case BUTTON_0:
        return "0";
    case BUTTON_1:
        return "1";
    case BUTTON_2:
        return "2";
    case BUTTON_3:
        return "3";
    case BUTTON_4:
        return "4";
    case BUTTON_5:
        return "5";
    case BUTTON_6:
        return "6";
    case BUTTON_7:
        return "7";
    case BUTTON_8:
        return "8";
    case BUTTON_9:
        return "9";
    case BUTTON_PLUS:
        return "PLUS";
    case BUTTON_MINUS:
        return "MINUS";
    case BUTTON_EQ:
        return "EQ";
    case BUTTON_U_SD:
        return "U/SD";
    case BUTTON_CYCLE:
        return "CYCLE";
    case BUTTON_PLAY_PAUSE:
        return "PLAY/PAUSE";
    case BUTTON_BACKWARD:
        return "BACKWARD";
    case BUTTON_FORWARD:
        return "FORWARD";
    case BUTTON_POWER:
        return "POWER";
    case BUTTON_MUTE:
        return "MUTE";
    case BUTTON_MODE:
        return "MODE";
    case BUTTON_UNKNOWN_OR_ERROR:
        return "UNKNOWN/ERROR";
    default:
        return "UNMAPPED";
    }
}

void ir_decoder_task(void* pvParameters) {
    (void)pvParameters;

    decoder_task_init();

    while (1) {
        if (xSemaphoreTake(xSignaler, portMAX_DELAY) == pdTRUE) {
            ir_result_t decoded_signal;
            ir_decode(&decoded_signal);

            switch (decoded_signal.type) {
            case IR_FRAME_TYPE_DATA:
                decode_key_value(&decoded_signal);
                ESP_LOGI(TAG, "Command Received: %s", get_button_name(decoded_signal.button));
                if (decoded_signal.button == BUTTON_FORWARD) {
                    dht11_notify_read();
                } else if (decoded_signal.button == BUTTON_CYCLE) {
                    lcd_cycle_mode();
                } else if (decoded_signal.button == BUTTON_EQ) {
                    speaker_play_sound();
                }
                break;

            case IR_FRAME_TYPE_REPEAT:
                ESP_LOGI(TAG, "Repeat Code Detected");
                break;

            case IR_FRAME_TYPE_INVALID:
                ESP_LOGW(TAG, "Invalid Frame Detected");
                break;
            }
        }
    }
}