// main.c

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "sdkconfig.h"
#include "freertos/semphr.h"

#include "timeset.h"
#include "dht11_task.h"
#include "lcd_task.h"
#include "wifi.h"
#include "webserver.h"
#include "button.h"
#include "statusled.h"
#include "irdecoder.h"

#define DHT11_TASK_PRIORITY 15
#define LCD_TASK_PRIORITY 10
#define BUTTON_TASK_PRIORITY 12
#define IR_DECODER_TASK_PRIORITY 9

static const char* TAG = "APP_MAIN"; 
TaskHandle_t dht11_task_handle = NULL;
TaskHandle_t lcd_task_handle = NULL;
TaskHandle_t button_task_handle = NULL;
TaskHandle_t ir_decoder_task_handle = NULL;

void app_main(void) {
    ESP_LOGI(TAG, "Application Starting");
    status_led_init();
    status_led_set_state(STATUS_LED_STATE_STARTING);

    ESP_ERROR_CHECK(wifi_driver_start_and_connect("YadaWiFi", "ted785tip9109coat"));

    EventGroupHandle_t wifi_event_group = wifi_driver_get_event_group();
    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "WiFi event group not initialized!");
        return;
    }
    ESP_LOGI(TAG, "Waiting for WiFi connection...");

    status_led_set_state(STATUS_LED_STATE_IN_PROGRESS);

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    
    xDHT11Mutex = xSemaphoreCreateMutex();
    if (xDHT11Mutex == NULL) {
        ESP_LOGE("APP_MAIN", "Failed to create DHT11 data mutex! System may be unstable.");
        return;
    }

    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "WiFi connection failed after multiple retries. Cannot proceed.");
        status_led_set_state(STATUS_LED_STATE_ERROR);
        return;
    }
        
    ESP_LOGI(TAG, "WiFi connected successfully!\n");
    vTaskDelay(pdMS_TO_TICKS(5000)); 
    setup_time();

    BaseType_t xReturnedPinned = xTaskCreatePinnedToCore(dht11_read_task, "DHT11 Reader", 4096, NULL, DHT11_TASK_PRIORITY, &dht11_task_handle, 1);

    if (xReturnedPinned != pdPASS) {
        ESP_LOGE(TAG, "Failed to create DHT11 reading task!");
        return;
    } else {
        ESP_LOGI(TAG, "DHT11 reading task created with priority %d. Handle %p", DHT11_TASK_PRIORITY, dht11_task_handle);
    }

    httpd_handle_t server = start_webserver();
    if (server == NULL) {
        ESP_LOGE(TAG, "Server start unsuccessful");
        return;
    }
    ESP_LOGI(TAG, "Server start successful");
    

    BaseType_t xReturned = xTaskCreate(lcd_display_task, "LCD Displayer", 4096, NULL, LCD_TASK_PRIORITY, &lcd_task_handle);

    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LCD Display Task");
    } else {
        ESP_LOGI(TAG, "LCD display task created with priority %d", LCD_TASK_PRIORITY);
    }

    xReturned = xTaskCreate(button_press_task, "Button Task", 2048, NULL, BUTTON_TASK_PRIORITY, &button_task_handle);

    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Button task");
    } else {
        ESP_LOGI(TAG, "Button task created with priority %d", BUTTON_TASK_PRIORITY);
    }
    
    xReturned = xTaskCreate(ir_decoder_task, "IR Decoder Task", 4096, NULL, IR_DECODER_TASK_PRIORITY, &ir_decoder_task_handle);

    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Button task");
    } else {
        ESP_LOGI(TAG, "Button task created with priority %d", BUTTON_TASK_PRIORITY);
    }

    status_led_set_state(STATUS_LED_STATE_READY);
}