// button.c

#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "lcd_task.h"
#include "dht11_task.h"

static const char* TAG = "BUTTON_DRIVER";
static volatile TickType_t last_isr_tick = 0;
static SemaphoreHandle_t xSignaler = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    TickType_t current_tick = xTaskGetTickCountFromISR();

    if (current_tick - last_isr_tick > pdMS_TO_TICKS(DEBOUNCE_TIME_MS)) {
        last_isr_tick = current_tick;
        BaseType_t higher_priority_task = pdFALSE;
        xSemaphoreGiveFromISR(xSignaler, &higher_priority_task);

        if(higher_priority_task == pdTRUE) {
            portYIELD_FROM_ISR();
        }
    }
}

static void button_task_init() {
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_NEGEDGE);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_DISABLE);
    xSignaler = xSemaphoreCreateBinary();
    if (xSignaler == NULL) {
        ESP_LOGE(TAG, "FAILED TO CREATE SEMAPHORE: EXPECT UNSTABLE BEHAVIOR");
        return;
    }
    esp_err_t isr_service_result = gpio_install_isr_service(0);
    if (isr_service_result != ESP_OK && isr_service_result != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(isr_service_result));
        return;
    }
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, NULL));
}

void button_press_task(void* pvParameters) {
    (void)pvParameters;

    button_task_init();

    while (1) {
        if (xSemaphoreTake(xSignaler, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "BUTTON PRESSED");
            lcd_cycle_mode();
        }
    }
}