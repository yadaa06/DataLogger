// dht11_task.c

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "dht11_task.h"
#include "dht11.h"

static const char* TAG = "DHT11_TASK";

static float g_temperature = 0.0f;
static float g_humidity = 0.0f;

SemaphoreHandle_t xDHT11Mutex;

float dht11_get_temperature() {
    float temp_read = 0.0f;

    if (xSemaphoreTake(xDHT11Mutex, portMAX_DELAY) == pdTRUE) {
        temp_read = g_temperature;
        xSemaphoreGive(xDHT11Mutex);
    } else {
        ESP_LOGE(TAG, "ERROR: dht11_get_temperature failed to take mutex!");
    }
    return temp_read;
} 

float dht11_get_humidity(){
    float hum_read = 0.0f;

    if (xSemaphoreTake(xDHT11Mutex, portMAX_DELAY) == pdTRUE) {
        hum_read = g_humidity;
        xSemaphoreGive(xDHT11Mutex);
    } else {
       ESP_LOGE(TAG, "ERROR: dht11_get_humidity failed to take mutex!"); 
    }
    return hum_read;
}

void dht11_read_task(void *pvParameters) {
    (void)pvParameters;

    ESP_LOGI(TAG, "DHT11 reading task started");
    int suppress_fail_count = 3; 

    while(1) {
        float temp_c = 0.0f;
        float hum_c = 0.0f;

        bool suppress_driver_logs = (suppress_fail_count > 0);
        esp_err_t ret = read_dht_data(&temp_c, &hum_c, suppress_driver_logs);

        if (ret != ESP_OK) {
            if (suppress_fail_count > 0) {
                ESP_LOGW(TAG, "DHT11 read failed, retrying(%d suppressed attempts left)", suppress_fail_count);
                suppress_fail_count--; 
            } else {
                ESP_LOGE(TAG, "FAILED TO READ DHT11 DATA: %s", esp_err_to_name(ret));
            }
        }
        else {
            if (xSemaphoreTake(xDHT11Mutex, portMAX_DELAY) == pdTRUE) {
                g_temperature = temp_c * (9.0 / 5.0) + 32;
                g_humidity = hum_c;

                xSemaphoreGive(xDHT11Mutex);
            } else {
                ESP_LOGE(TAG, "ERROR: dht11 read task failed to take mutex");
            }

            ESP_LOGI(TAG, "Temperature: %.2f F, Humidity: %.1f %%", g_temperature, g_humidity);
            suppress_fail_count = 3; 
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}