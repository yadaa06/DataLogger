// irdecoder.c

#include <string.h>
#include <math.h>
#include "irdecoder.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"

static const char* TAG = "IR_DRIVER";
static uint64_t last_time = 0;
static volatile uint8_t active_idx = 0;
static volatile uint32_t ir_buffer_A[IR_TIMES_SIZE];
static volatile uint32_t ir_buffer_B[IR_TIMES_SIZE];
static volatile uint32_t *active_buffer_add = ir_buffer_A;
static volatile uint32_t *decode_buffer_add = ir_buffer_B;
static volatile uint8_t decode_len = 0;
static SemaphoreHandle_t xSignaler = NULL;

gptimer_handle_t GPtimer;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint64_t curr_time, pulse_length;
    gptimer_get_raw_count(GPtimer, &curr_time);

    if (last_time == 0) {
        last_time = curr_time;
        return;
    }

    pulse_length = curr_time - last_time;
    last_time = curr_time;

    if (pulse_length > 15000) {
        if (active_idx > 0) {
            decode_len = active_idx;
            volatile uint32_t *tmp = active_buffer_add;
            active_buffer_add = decode_buffer_add;
            decode_buffer_add = tmp;

            BaseType_t higher_priority_task = pdFALSE;
            xSemaphoreGiveFromISR(xSignaler, &higher_priority_task);
        }
        active_idx = 0;
    } else {
        if (active_idx < IR_TIMES_SIZE) {
            active_buffer_add[active_idx++] = pulse_length;
        } else {
            last_time = 0;
            active_idx = 0;
        }
    }
}

static void decoder_task_init() {
    gpio_reset_pin(IR_PIN);
    gpio_set_direction(IR_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(IR_PIN, GPIO_INTR_ANYEDGE);
    
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000
    };

    xSignaler = xSemaphoreCreateBinary();
    if (xSignaler == NULL) {
        ESP_LOGE(TAG, "FAILED TO CREATE SEMAPHORE: UNSTABLE BEHAVIOR EXPECTED");
        return;
    }

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &GPtimer));
    
    esp_err_t isr_service_result = gpio_install_isr_service(0);
    if (isr_service_result != ESP_OK && isr_service_result != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(isr_service_result));
        return;
    }
    ESP_ERROR_CHECK(gpio_isr_handler_add(IR_PIN, gpio_isr_handler, NULL));

    gptimer_enable(GPtimer);
    gptimer_start(GPtimer);
}

static void ir_decode(ir_result_t *result) {
    result->type = IR_FRAME_TYPE_INVALID;
    result->address = 0;
    result->command = 0;

    if (decode_len > 50) {
        if (!IR_MATCH(decode_buffer_add[0], NEC_START_PULSE) || !IR_MATCH(decode_buffer_add[1], NEC_START_SPACE)) {
            ESP_LOGE(TAG, "IR SIGNAL doesn't follow NEC start protocol");
            return;
        }

        uint32_t decoded_data = 0;
        for (int i = 0; i < 32; i++) {
            uint32_t pulse = decode_buffer_add[i * 2 + 2];
            uint32_t space = decode_buffer_add[i * 2 + 3];

            if (!IR_MATCH(pulse, NEC_BIT_PULSE)) {
                ESP_LOGE(TAG, "IR bit not a valid bit pulse");
                return;
            }

            decoded_data <<= 1;
            if (IR_MATCH(space, NEC_ONE_SPACE)) {
                decoded_data |= 1;
            } else if (IR_MATCH(space, NEC_ZERO_SPACE)) {
                decoded_data |= 0;
            } else {
                ESP_LOGE(TAG, "IR bit not a valid bit space");
                return;
            }
        }

        uint8_t addr = (decoded_data >> 24) & 0xFF;
        uint8_t inv_addr = (decoded_data >> 16) & 0xFF;
        uint8_t cmd = (decoded_data >> 8) & 0xFF;
        uint8_t inv_cmd = (decoded_data) & 0xFF;

        if ((addr ^ inv_addr) != 0xFF || (cmd ^ inv_cmd) != 0xFF) {
            ESP_LOGE(TAG, "Checksum Failed");
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

void ir_decoder_task(void* pvParameters) {
    (void)pvParameters;

    decoder_task_init();

    while (1) {
        if (xSemaphoreTake(xSignaler, portMAX_DELAY) == pdTRUE) {
                ir_result_t decoded_signal;
                ir_decode(&decoded_signal);

                switch (decoded_signal.type) {
                case IR_FRAME_TYPE_DATA:
                    ESP_LOGI(TAG, "Data Frame! Address: 0x%02X, Command: 0x%02X",
                             decoded_signal.address, decoded_signal.command);
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