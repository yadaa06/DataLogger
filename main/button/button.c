// button.c

#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "lcd_task.h"
#include "dht11_task.h"

static const char* TAG = "BUTTON_DRIVER";
SemaphoreHandle_t xSignaler = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    BaseType_t higher_priority_task = pdFALSE;

    xSemaphoreGiveFromISR(xSignaler, &higher_priority_task);

    if(higher_priority_task == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void button_task_init() {
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_NEGEDGE);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_DISABLE);
    xSignaler = xSemaphoreCreateBinary();
    if (xSignaler == NULL) {
        ESP_LOGE(TAG, "FAILED TO CREATE SEMAPHORE: EXPECT UNSTABLE BEHAVIOR");
        return;
    }
    ESP_LOGI(TAG, "Semaphore Created");
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, NULL);
}

void button_press_task(void* pvParameters) {
    (void)pvParameters;

    while (1) {
        if (xSemaphoreTake(xSignaler, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "BUTTON PRESSED");
            lcd_cycle_mode();
        }
    }
}