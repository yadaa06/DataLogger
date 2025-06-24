// main.c

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "sdkconfig.h"

#include "dht11_task.h"
#include "lcd_task.h"
#include "wifi.h"
#include "webserver.h"


#define DHT11_TASK_PRIORITY 15
#define LCD_TASK_PRIORITY 10

static const char* TAG = "APP_MAIN"; 

void app_main(void) {
    ESP_LOGI(TAG, "Application Starting"); 
    esp_err_t ret;

    ret = wifi_driver_start_and_connect("YadaWiFi", "ted785tip9109coat");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start and connect WiFi: %s", esp_err_to_name(ret));
        return;
    }

    EventGroupHandle_t wifi_event_group = wifi_driver_get_event_group();
    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "WiFi event group not initialized!");
        return;
    }
    ESP_LOGI(TAG, "Waiting for WiFi connection...");

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    
    xDHT11Mutex = xSemaphoreCreateMutex();
    if (xDHT11Mutex == NULL) {
        ESP_LOGE("APP_MAIN", "Failed to create DHT11 data mutex! System may be unstable.");
        return;
    } else {
        ESP_LOGI("APP_MAIN", "DHT11 data mutex created successfully.");
    }
        
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi connected successfully!\n");
        vTaskDelay(pdMS_TO_TICKS(5000)); 
        httpd_handle_t server = start_webserver();
        if (server == NULL) {
            ESP_LOGE(TAG, "Server start unsuccessful");
            return;
        }
        ESP_LOGI(TAG, "Server start successful");
       
        BaseType_t xReturnedPinned = xTaskCreatePinnedToCore(
            dht11_read_task, 
            "DHT11 Reader", 
            4096, 
            NULL, 
            DHT11_TASK_PRIORITY, 
            NULL,
            1
        );
        if (xReturnedPinned != pdPASS) {
            ESP_LOGE(TAG, "Failed to create DHT11 reading task!");
            return;
        } else {
            ESP_LOGI(TAG, "DHT11 reading task created with priority %d", DHT11_TASK_PRIORITY);
        }

        BaseType_t xReturned = xTaskCreate(
            lcd_display_task,
            "LCD Displayer",
            4096,
            NULL,
            LCD_TASK_PRIORITY,
            NULL
        );

        if (xReturned != pdPASS) {
            ESP_LOGE(TAG, "Failed to create LCD Display Task");
        } else {
            ESP_LOGI(TAG, "LCD display task created with priority %d", LCD_TASK_PRIORITY);
        }

    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "WiFi connection failed after multiple retries. Cannot proceed.");
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT during WiFi wait!");
    }
}