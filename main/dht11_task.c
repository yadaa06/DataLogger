// dht11_task.c

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "dht11_task.h"
#include "dht11.h"

static const char* DHT_TASK_TAG = "DHT11_TASK";

static float g_temperature = 0.0f;
static float g_humidity = 0.0f;

float dht11_get_temperature() {
    return g_temperature;
} 

float dht11_get_humidity(){
    return g_humidity;
}

void dht11_read_task(void *pvParameters) {
    (void)pvParameters;

    ESP_LOGI(DHT_TASK_TAG, "DHT11 reading task started");
    int suppress_fail_count = 3; 

    while(1) {
        float temp_c = 0.0f;
        float hum_c = 0.0f;

        bool suppress_driver_logs = (suppress_fail_count > 0);
        esp_err_t ret = read_dht_data(&temp_c, &hum_c, suppress_driver_logs);

        if (ret != ESP_OK) {
            if (suppress_fail_count > 0) {
                ESP_LOGW(DHT_TASK_TAG, "DHT11 read failed, retrying(%d suppressed attempts left)", suppress_fail_count);
                suppress_fail_count--; 
            } else {
                ESP_LOGE(DHT_TASK_TAG, "FAILED TO READ DHT11 DATA: %s", esp_err_to_name(ret));
            }
        }
        else {
            g_temperature = temp_c * (9.0 / 5.0) + 32;
            g_humidity = hum_c;
            ESP_LOGI(DHT_TASK_TAG, "Temperature: %.1f F, Humidity: %.1f %%", g_temperature, g_humidity);
            suppress_fail_count = 3; 
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}