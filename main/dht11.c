// dht11.c

#include "dht11.h"     
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "rom/ets_sys.h" // For esp_rom_delay_us 

// Internal defines and static variables for this module
static const char* TAG = "DHT11_DRIVER"; 

esp_err_t read_dht_data(float *temperature, float *humidity){
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // 1. Send start signal
    gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_PIN, 0);
    esp_rom_delay_us(18000);
    gpio_set_level(DHT11_PIN, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);


    // 2. DHT Response
    uint64_t start_time = esp_timer_get_time();

    while(gpio_get_level(DHT11_PIN) == 1) {
        if (esp_timer_get_time() - start_time > 100) {
            ESP_LOGE(TAG, "DHT FAILED TO PULL SIGNAL DOWN (DURING SETUP)");
            return ESP_FAIL;
        }
    }

    start_time = esp_timer_get_time();
    while(gpio_get_level(DHT11_PIN) == 0) {
        if (esp_timer_get_time() - start_time > 100) {
            ESP_LOGE(TAG, "DHT FAILED TO PULL SIGNAL BACK UP (DURING SETUP)");
            return ESP_FAIL;
        }
    }

    start_time = esp_timer_get_time();
    while(gpio_get_level(DHT11_PIN) == 1) {
        if (esp_timer_get_time() - start_time > 100) {
            ESP_LOGE(TAG, "DHT FAILED TO PULL SIGNAL BACK DOWN (BEGININING OF DATA)");
            return ESP_FAIL;
        }
    }

    // 3. Data Transmission
    for (uint8_t i = 0; i < 5; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            start_time = esp_timer_get_time();
            while(gpio_get_level(DHT11_PIN) == 0) {
                if (esp_timer_get_time() - start_time > 70) {
                    ESP_LOGE(TAG, "DHT FAILED TO PULL SIGNAL BACK UP (DURING DATA)");
                    return ESP_FAIL;
                }
            }
            start_time = esp_timer_get_time();
            while(gpio_get_level(DHT11_PIN) == 1) {
                if (esp_timer_get_time() - start_time > 120) {
                    ESP_LOGE(TAG, "DHT FAILED TO PULL SIGNAL BACK DOWN (DURING DATA)");
                    return ESP_FAIL;
                }
            }

            uint64_t pulse_duration = esp_timer_get_time() - start_time;
            data[i] <<= 1;
                if (pulse_duration > 40) {
                    data[i] |= 1;
                }
        }
    }

    // 4. Checksum
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        ESP_LOGI(TAG, "THIS ROUND OF DATA IS VALID");
        *humidity = (float)data[0] + (float)data[1] / 10.0f;
        *temperature = (float)data[2] + (float)data[3] / 10.0f;
        return ESP_OK;
    }
    else {
        ESP_LOGE(TAG, "CHECKSUM FAILED");
        return ESP_FAIL;
    }
}