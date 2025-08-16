// lcd_task.c

#include "lcd_task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "dht11_task.hpp"
#include "lcd_i2c.h"

static const char* TAG = "LCD_TASK";
static lcd_mode_t current_mode = LCD_MODE_TEMP;
static TaskHandle_t lcd_display_task_handle = NULL;

void lcd_cycle_mode(void) {
    if (lcd_display_task_handle != NULL) {
        xTaskNotify(lcd_display_task_handle, BUTTON_UL_VALUE, eSetValueWithoutOverwrite);
    }
}

void lcd_display_task(void *pvParameters) {
    (void)pvParameters;
    ESP_LOGI(TAG, "Starting LCD TASK");

    lcd_display_task_handle = xTaskGetCurrentTaskHandle();

    lcd_i2c_handle_t* lcd_handle = lcd_i2c_init();
    if (lcd_handle == NULL) {
        ESP_LOGE(TAG, "LCD INITIALIZATION FAILED");
        return;
    }

    ESP_LOGI(TAG, "LCD INITIALIZED");
    
    while(1) {
        uint32_t ulNotifiedValue;
        BaseType_t xResult = xTaskNotifyWait(0, UINT32_MAX, &ulNotifiedValue, pdMS_TO_TICKS(5000));

        if (xResult == pdTRUE && ulNotifiedValue == BUTTON_UL_VALUE) {
            ESP_LOGI(TAG, "Button Pressed, Changing Mode");
            current_mode = (current_mode + 1) % LCD_MODE_MAX;
        }

        vTaskDelay(pdMS_TO_TICKS(2));
        lcd_i2c_clear(lcd_handle);
        vTaskDelay(pdMS_TO_TICKS(2));
        lcd_i2c_home(lcd_handle);
        vTaskDelay(pdMS_TO_TICKS(2));

        switch (current_mode) {
            case LCD_MODE_TEMP:
                float temperature = dht11_get_temperature();
                lcd_i2c_write_string(lcd_handle, "Temp: %.2f %cF", temperature, 223);
                lcd_i2c_set_cursor(lcd_handle, 0, 1);
                lcd_i2c_write_string(lcd_handle, "Next: Hum");
                break;
            case LCD_MODE_HUM:
                float humidity = dht11_get_humidity();
                lcd_i2c_write_string(lcd_handle, "Hum: %.2f %%", humidity);
                lcd_i2c_set_cursor(lcd_handle, 0, 1);
                lcd_i2c_write_string(lcd_handle, "Next: Last Read");
                break;
            case LCD_MODE_LAST_READ:
                uint64_t last_read_us = dht11_get_last_read();
                uint64_t current_time_us = esp_timer_get_time();
                uint32_t seconds_since_last_read = (current_time_us - last_read_us) / 1000000;
                lcd_i2c_write_string(lcd_handle, "LR: %lu secs ago", seconds_since_last_read);
                lcd_i2c_set_cursor(lcd_handle, 0, 1);
                lcd_i2c_write_string(lcd_handle, "Next: Temp");
                break;
            default:
                break;
        }
    }
}