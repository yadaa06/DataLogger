// main.c

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "sdkconfig.h"

#include "dht11_task.h"
#include "lcd_i2c.h"
#include "wifi.h"
#include "webserver.h"

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
    
    // xDHT11Mutex = xSemaphoreCreateMutex();
    // if (xDHT11Mutex == NULL) {
    //     ESP_LOGE("APP_MAIN", "Failed to create DHT11 data mutex! System may be unstable.");
    //     return;
    // } else {
    //     ESP_LOGI("APP_MAIN", "DHT11 data mutex created successfully.");
    // }
        
    lcd_i2c_handle_t* lcd_handle = lcd_i2c_init();
    if (lcd_handle == NULL) {
        ESP_LOGE(APP_TAG, "LCD INITIALIZATION FAILED");
        return;
    }
    ESP_LOGI(APP_TAG, "LCD INITIALIZED");

    lcd_i2c_write_string(lcd_handle, "Hello World!");

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(APP_TAG, "WiFi connected successfully!\n");
        vTaskDelay(pdMS_TO_TICKS(5000)); 
        httpd_handle_t server = start_webserver();
        if (server == NULL) {
            ESP_LOGE(APP_TAG, "Server start unsuccessful");
            return;
        }
        ESP_LOGI(APP_TAG, "Server start successful");
       
        BaseType_t xReturned = xTaskCreatePinnedToCore(
            dht11_read_task, 
            "DHT11 Reader", 
            4096, 
            NULL, 
            15, 
            NULL,
            1
        );
        if (xReturned != pdPASS) {
            ESP_LOGE(APP_TAG, "Failed to create DHT11 reading task!");
            return;
        } else {
        ESP_LOGI(APP_TAG, "DHT11 reading task created with priority %d.", 15);
        }
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(APP_TAG, "WiFi connection failed after multiple retries. Cannot proceed.");
    } else {
        ESP_LOGE(APP_TAG, "UNEXPECTED EVENT during WiFi wait!");
    }
}