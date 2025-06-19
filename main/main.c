// main.c

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "dht11.h"
#include "lcd_i2c.h"

static const char* APP_TAG = "MAIN_APP"; 

void app_main(void)
{
    ESP_LOGI(APP_TAG, "Application Starting"); 
    vTaskDelay(pdMS_TO_TICKS(DHT11_POWER_ON_DELAY_MS)); 

    i2c_master_bus_handle_t i2c_bus = lcd_i2c_master_init();
    if (i2c_bus == NULL) {
        ESP_LOGE(APP_TAG, "FAILED TO INITIALIZE I2C BUS");
        return;
    }

    ESP_LOGI(APP_TAG, "INITIALIZED I2C BUS");
    lcd_i2c_handle_t *lcd_handle = lcd_i2c_create(i2c_bus, LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
    if (lcd_handle == NULL) {
        ESP_LOGE(APP_TAG, "Failed to create LCD handle!");
        return;
    }
    ESP_LOGI(APP_TAG, "LCD handle created");

    lcd_i2c_init(lcd_handle);
    ESP_LOGI(APP_TAG, "LCD initialization sequence finished.");



    float temperature = 0.0f;
    float humidity = 0.0f;

    while (1) {
        esp_err_t ret = read_dht_data(&temperature, &humidity);

        if (ret == ESP_OK) {
            ESP_LOGI(APP_TAG, "Temperature: %.1f C, Humidity: %.1f %%\n", temperature, humidity);
        } else {
            ESP_LOGE(APP_TAG, "Failed to read DHT_11 Data");
        }

        ret = lcd_i2c_clear(lcd_handle);

        if (ret == ESP_OK) {
            ESP_LOGI(APP_TAG, "Screen Cleared Successfully");
        } else {
            ESP_LOGE(APP_TAG, "Failed to Clear Screen");
        }

        ret = lcd_i2c_write_string(lcd_handle, "Temp: %.2f C", temperature);
        if (ret == ESP_OK) {
            ESP_LOGI(APP_TAG, "Temperature Displayed Successfully");
        } else {
            ESP_LOGE(APP_TAG, "Failed to display temperature");
        }

        lcd_i2c_set_cursor(lcd_handle, 0, 1);
        ret = lcd_i2c_write_string(lcd_handle, "Humidity: %.1f %%", humidity);
        if (ret == ESP_OK) {
            ESP_LOGI(APP_TAG, "Humidity Displayed Successfully");
        } else {
            ESP_LOGE(APP_TAG, "Failed to display humidity");
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}