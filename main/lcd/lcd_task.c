// lcd_task.c

#include "lcd_task.h"
#include "lcd_i2c.h"
#include "dht11_task.h"
#include "esp_log.h"

static const char* TAG = "LCD_TASK";
static lcd_mode_t current_mode = LCD_MODE_TEMP;
static TaskHandle_t lcd_display_task_handle = NULL;

void lcd_cycle_mode(void) {
    if (lcd_display_task_handle != NULL) {
        xTaskNotify(lcd_display_task_handle, 1, eSetValueWithoutOverwrite);
    }
}

void lcd_display_task(void *pvParameters) {
    (void)pvParameters;
    ESP_LOGI(TAG, "Starting LCD TASK");

    lcd_display_task_handle = xTaskGetCurrentTaskHandle();
    TickType_t last_read_time = xTaskGetTickCount();

    lcd_i2c_handle_t* lcd_handle = lcd_i2c_init();
    if (lcd_handle == NULL) {
        ESP_LOGE(TAG, "LCD INITIALIZATION FAILED");
        return;
    }

    ESP_LOGI(TAG, "LCD INITIALIZED");
    
    while(1) {
        uint32_t ulNotifiedValue;
        BaseType_t xResult = xTaskNotifyWait(0, UINT32_MAX, &ulNotifiedValue, pdMS_TO_TICKS(60000));

        if (xResult == pdTRUE && ulNotifiedValue > 0) {
            ESP_LOGI(TAG, "Button Pressed, Changing Mode");
            current_mode = (current_mode + 1) % LCD_MODE_MAX;
        }

        float temperature = dht11_get_temperature();
        float humidity = dht11_get_humidity();
        last_read_time = xTaskGetTickCount();

        lcd_i2c_clear(lcd_handle);
        lcd_i2c_home(lcd_handle);

        switch (current_mode) {
            case LCD_MODE_TEMP:
                lcd_i2c_write_string(lcd_handle, "Temp: %.2f %cF", temperature, 223);
                lcd_i2c_set_cursor(lcd_handle, 0, 1);
                lcd_i2c_write_string(lcd_handle, "Next: Hum");
                break;
            case LCD_MODE_HUM:
                lcd_i2c_write_string(lcd_handle, "Hum: %.2f %%", humidity);
                lcd_i2c_set_cursor(lcd_handle, 0, 1);
                lcd_i2c_write_string(lcd_handle, "Next: Last Read");
                break;
            case LCD_MODE_LAST_READ:
                TickType_t current_time = xTaskGetTickCount();
                uint32_t seconds_since_last_read = (current_time - last_read_time) * (portTICK_PERIOD_MS / 1000);
                lcd_i2c_write_string(lcd_handle, "LR: %lu secs", seconds_since_last_read);
                lcd_i2c_set_cursor(lcd_handle, 0, 1);
                lcd_i2c_write_string(lcd_handle, "Next: Temp");
                break;
            default:
                break;
        }
    }
}