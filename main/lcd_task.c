// lcd_task.c

#include "lcd_task.h"
#include "lcd_i2c.h"
#include "dht11_task.h"
#include "esp_log.h"

static const char* TAG = "LCD_TASK";


void lcd_display_task(void *pvParameters) {

    ESP_LOGI(TAG, "Starting LCD TASK");

    lcd_i2c_handle_t* lcd_handle = lcd_i2c_init();
    if (lcd_handle == NULL) {
        ESP_LOGE(TAG, "LCD INITIALIZATION FAILED");
        return;
    }

    ESP_LOGI(TAG, "LCD INITIALIZED");
    
    while(1) {
        esp_err_t ret = ESP_OK;
        (void)pvParameters;

        float temperature = dht11_get_temperature();
        float humidity = dht11_get_humidity();

        ret = lcd_i2c_clear(lcd_handle);
        if (ret != ESP_OK) {
            return;
        }
        ret = lcd_i2c_home(lcd_handle);
        if (ret != ESP_OK) {
            return;
        }
        ret = lcd_i2c_write_string(lcd_handle, "Temp: %.2f F", temperature);
        if (ret != ESP_OK) {
            return;
        }
        ret = lcd_i2c_set_cursor(lcd_handle, 0, 1);
        if (ret != ESP_OK) {
            return;
        } 
        ret = lcd_i2c_write_string(lcd_handle, "Hum: %.1f %%", humidity);
        if (ret != ESP_OK) {
            return;
        } 
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}