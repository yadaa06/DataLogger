// dht11_task.c

#include <stdbool.h>
#include <time.h>
#include "dht11_task.h"
#include "dht11.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "DHT11_TASK";
SemaphoreHandle_t xDHT11Mutex = NULL;

static float g_temperature = 0.0f;
static float g_humidity = 0.0f;

static dht11_reading_t dht_history[DHT_HISTORY_SIZE];
static int history_idx = 0;
static int num_history_readings = 0;
static uint64_t last_successful_read = 0;


void dht11_get_history(dht11_reading_t *history_buffer, uint32_t *web_num_readings) {
    if (xSemaphoreTake(xDHT11Mutex, portMAX_DELAY) == pdTRUE) {
        *web_num_readings = num_history_readings;

        if (num_history_readings > 0) {
            int start_idx = (history_idx - num_history_readings + DHT_HISTORY_SIZE) % DHT_HISTORY_SIZE;
            
            for (int i = 0; i < num_history_readings; i++) {
                int current_buffer_idx = (start_idx + i) % DHT_HISTORY_SIZE;
                history_buffer[i] = dht_history[current_buffer_idx];
            }
        }

        xSemaphoreGive(xDHT11Mutex);
    } else {
        ESP_LOGE(TAG, "ERROR: dht11_get_history failed to take mutex!");
        *web_num_readings = 0;
    }
}

uint64_t dht11_get_last_read() {
    uint64_t time_read = 0;
    if (xSemaphoreTake(xDHT11Mutex, portMAX_DELAY) == pdTRUE) {
        time_read = last_successful_read;
        xSemaphoreGive(xDHT11Mutex);
    }
    return time_read;
}

void dht11_notify_read() {
    if (dht11_task_handle != NULL) {
        xTaskNotifyGive(dht11_task_handle);
        ESP_LOGI(TAG, "Sent notification to DHT11 task to read NOW");
    } else {
        ESP_LOGE(TAG, "DHT11 TASK HANDLE IS NULL, CAN'T SEND NOTIF");
    }
}


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
    float temp_c = 0.0f;
    float hum_c = 0.0f;

    uint64_t last_read_attempt_time = 0;
    read_dht_data(&temp_c, &hum_c, true);
    last_read_attempt_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Dummy reading");
    vTaskDelay(3000);

    while(1) {
        esp_err_t ret;
        uint64_t current_time_us = esp_timer_get_time();
        uint64_t time_since_last_read = current_time_us - last_read_attempt_time;

        if (time_since_last_read < MIN_READ_INTERVAL_US) {
            ESP_LOGW(TAG, "DHT11 read requested too soon (%.2f ms since last read). Waiting.", (float)time_since_last_read / 1000.0f);

            uint64_t remaining_wait = MIN_READ_INTERVAL_US - time_since_last_read;
            TickType_t remaining_ticks = pdMS_TO_TICKS(remaining_wait / 1000);
            if (remaining_ticks == 0 && remaining_wait == 0) {
                remaining_ticks = 1;
            }

            xTaskNotifyWait(0, 0, NULL, remaining_ticks);

            continue;
        }
        last_read_attempt_time = esp_timer_get_time();

        for (int attempts = 1; attempts <= MAXATTEMPTS; attempts++) {
            bool suppress_driver_logs = (attempts < MAXATTEMPTS);
            ret = read_dht_data(&temp_c, &hum_c, suppress_driver_logs);

            if (ret != ESP_OK) {
                ESP_LOGW(TAG, "DHT11 read attempt failed, retrying (%d/%d)", attempts, MAXATTEMPTS);
                vTaskDelay(pdMS_TO_TICKS(3000));
            } else {
                break;
            }
        }

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "CRITICAL ERROR, FAILED TO READ DHT11 DATA 3 TIMES");
        }
        else {
            if (xSemaphoreTake(xDHT11Mutex, portMAX_DELAY) == pdTRUE) {
                g_temperature = temp_c * (9.0 / 5.0) + 32;
                g_humidity = hum_c;

                dht_history[history_idx].temperature = g_temperature;
                dht_history[history_idx].humidity = g_humidity;
                dht_history[history_idx].timestamp = time(NULL);
                history_idx++;

                if (history_idx == DHT_HISTORY_SIZE) {
                    history_idx = 0;
                }
                if (num_history_readings < DHT_HISTORY_SIZE) {
                    num_history_readings++;
                }
                last_successful_read = esp_timer_get_time();

                xSemaphoreGive(xDHT11Mutex);
            } else {
                ESP_LOGE(TAG, "ERROR: dht11 read task failed to take mutex");
            }

            if (lcd_task_handle != NULL) {
                xTaskNotifyGive(lcd_task_handle);
                ESP_LOGI(TAG, "Notified LCD of new data");
            } else {
                ESP_LOGE(TAG, "LCD TASK HANDLE IS NULL, CAN'T NOTIFY");
            }

            ESP_LOGI(TAG, "Temperature: %.2f F, Humidity: %.1f %%", g_temperature, g_humidity);
        }
        xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(60000));
    }
}