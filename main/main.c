// main.c

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "dht11.h"
#include "lcd_i2c.h"
#include "wifi.h"

static const char* APP_TAG = "MAIN_APP"; 

void app_main(void) {
    ESP_LOGI(APP_TAG, "Application Starting"); 
    esp_err_t ret;

    ret = wifi_driver_start_and_connect("YadaWiFi", "ted785tip9109coat");
    if (ret != ESP_OK) {
        ESP_LOGE(APP_TAG, "Failed to start and connect WiFi: %s", esp_err_to_name(ret));
        return;
    }

    EventGroupHandle_t wifi_event_group = wifi_driver_get_event_group();
    if (wifi_event_group == NULL) {
        ESP_LOGE(APP_TAG, "WiFi event group not initialized!");
        return;
    }
    ESP_LOGI(APP_TAG, "Waiting for WiFi connection...");

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

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

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(APP_TAG, "WiFi connected successfully!\n");
        vTaskDelay(pdMS_TO_TICKS(5000)); 
        while(1) {
            esp_err_t ret = read_dht_data(&temperature, &humidity);

            if (ret == ESP_OK) {
                ESP_LOGI(APP_TAG, "Temperature: %.1f C, Humidity: %.1f %%\n", temperature, humidity);
            } else {
                ESP_LOGE(APP_TAG, "Failed to read DHT_11 Data");
            }

            ret = lcd_i2c_clear(lcd_handle);

            if (ret != ESP_OK) {
                ESP_LOGE(APP_TAG, "Failed to Clear Screen");
            }

            ret = lcd_i2c_write_string(lcd_handle, "Temp: %.2f C", temperature);
            if (ret != ESP_OK) {
                ESP_LOGE(APP_TAG, "Failed to display temperature");
            }

            lcd_i2c_set_cursor(lcd_handle, 0, 1);
            ret = lcd_i2c_write_string(lcd_handle, "Humidity: %.1f %%", humidity);
            if (ret != ESP_OK) {
                ESP_LOGE(APP_TAG, "Failed to display humidity");
            }

            vTaskDelay(pdMS_TO_TICKS(3000));
        }
                    
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(APP_TAG, "WiFi connection failed after multiple retries. Cannot proceed.");
    } else {
        ESP_LOGE(APP_TAG, "UNEXPECTED EVENT during WiFi wait!");
    }
}