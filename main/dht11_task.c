// dht11_task.c

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

    float init_temp = 0.0f;
    float init_hum = 0.0f;

    esp_log_level_set("DHT11_DRIVER", ESP_LOG_WARN);
    read_dht_data(&init_temp, &init_hum);
    esp_log_level_set("DHT11_DRIVER", ESP_LOG_ERROR);

    vTaskDelay(pdMS_TO_TICKS(5000));

    while(1) {
        float temp = 0.0f;
        float hum = 0.0f;

        esp_err_t ret = read_dht_data(&temp, &hum);
        if (ret != ESP_OK) {
            ESP_LOGE(DHT_TASK_TAG, "FAILED TO READ DHT11 DATA: %s", esp_err_to_name(ret));
        }
        else {
            g_temperature = temp;
            g_humidity = hum;
            ESP_LOGI(DHT_TASK_TAG, "Temperature: %.1f C, Humidity: %.1f %%", g_temperature, g_humidity);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
