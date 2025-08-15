// main.c

#include "button.h"
#include "dht11_task.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "irdecoder.h"
#include "lcd_task.h"
#include "speaker_driver.h"
#include "statusled.h"
#include "timeset.h"
#include "webserver.h"
#include "wifi.h"

#define DHT11_TASK_PRIORITY 15
#define LCD_TASK_PRIORITY 10
#define BUTTON_TASK_PRIORITY 12
#define IR_DECODER_TASK_PRIORITY 9
#define SPEAKER_TASK_PRIORITY 13

static const char* TAG = "APP_MAIN";

TaskHandle_t dht11_task_handle      = NULL;
TaskHandle_t lcd_task_handle        = NULL;
TaskHandle_t button_task_handle     = NULL;
TaskHandle_t ir_decoder_task_handle = NULL;
TaskHandle_t speaker_task_handle    = NULL;

void create_task_or_fail(TaskFunction_t task_func, const char* name, uint32_t stack, void* params, UBaseType_t priority, TaskHandle_t* handle) {
    BaseType_t result = xTaskCreate(task_func, name, stack, params, priority, handle);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create %s task", name);
    } else {
        ESP_LOGI(TAG, "%s task created with priority %d", name, priority);
    }
}

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

    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "WiFi connection failed after multiple retries. Cannot proceed.");
        status_led_set_state(STATUS_LED_STATE_ERROR);
        return;
    }

    ESP_LOGI(TAG, "WiFi connected successfully!\n");
    vTaskDelay(pdMS_TO_TICKS(5000));

    ESP_ERROR_CHECK(timeset_driver_start_and_wait());

    httpd_handle_t server = start_webserver();
    if (server == NULL) {
        ESP_LOGE(TAG, "Server start unsuccessful");
        return;
    }
    ESP_LOGI(TAG, "Server start successful");

    xDHT11Mutex = xSemaphoreCreateMutex();
    if (xDHT11Mutex == NULL) {
        ESP_LOGE("APP_MAIN", "Failed to create DHT11 data mutex! System may be unstable.");
        return;
    }
    
    create_task_or_fail(dht11_read_task, "DHT11 Reader", 4096, NULL, DHT11_TASK_PRIORITY, &dht11_task_handle);
    create_task_or_fail(lcd_display_task, "LCD Displayer", 4096, NULL, LCD_TASK_PRIORITY, &lcd_task_handle);
    create_task_or_fail(button_press_task, "Button Task", 2048, NULL, BUTTON_TASK_PRIORITY, &button_task_handle);
    create_task_or_fail(ir_decoder_task, "IR Decoder Task", 4096, NULL, IR_DECODER_TASK_PRIORITY, &ir_decoder_task_handle);
    create_task_or_fail(speaker_driver_play_task, "Speaker", 4096, NULL, SPEAKER_TASK_PRIORITY, &speaker_task_handle);

    status_led_set_state(STATUS_LED_STATE_READY);
}