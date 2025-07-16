// irdecoder.c

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
static volatile uint8_t currIndex = 0;
static volatile uint32_t ir_times[IR_TIMES_SIZE];
static SemaphoreHandle_t xSignaler = NULL;

gptimer_handle_t GPtimer;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint64_t curr_time, pulse_length;
    gptimer_get_raw_count(GPtimer, &curr_time);

    pulse_length = curr_time - last_time;
    last_time = curr_time;

    if (pulse_length > 5000) {
        if (currIndex > 10) {
            BaseType_t higher_priority_task = pdFALSE;
            xSemaphoreGiveFromISR(xSignaler, &higher_priority_task);
        }

        currIndex = 0;
    } else {
        if (currIndex < IR_TIMES_SIZE) {
            ir_times[currIndex++] = pulse_length;
        }
    }

}

static void decoder_task_init() {
    gpio_reset_pin(IR_PIN);
    gpio_set_direction(IR_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(IR_PIN, GPIO_INTR_ANYEDGE);
    gpio_set_pull_mode(IR_PIN, GPIO_PULLUP_ENABLE);
    
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

void ir_decoder_task(void* pvParameters) {
    (void)pvParameters;

    decoder_task_init();

    while (1) {
        if (xSemaphoreTake(xSignaler, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "IR signal received!");
        }
    }
}