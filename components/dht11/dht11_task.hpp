// dht11_task.hpp

#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "time.h"

#define DHT11_TASK_PRIORITY 15
#define DHT11_COOLDOWN 3000 
#define MAXATTEMPTS 3
#define MIN_READ_INTERVAL_US 3000000
#define DHT_HISTORY_SIZE 60

typedef struct {
    float temperature;
    float humidity;
    time_t timestamp;
} dht11_reading_t;

#ifdef __cplusplus
#include <cmath>

class DHT11Sensor {
private:
    SemaphoreHandle_t mutex = nullptr;
    float temperature = NAN;
    float humidity = NAN;
    TaskHandle_t taskHandle = nullptr;
    TaskHandle_t lcd_task_handle = nullptr;
    dht11_reading_t dht_history[DHT_HISTORY_SIZE];
    int history_idx = 0;
    int num_history_readings = 0;
    uint64_t last_successful_read = 0;

    void read_data_loop();
    static void read_data_task_wrapper(void* pvParameters);

public:
    explicit DHT11Sensor(TaskHandle_t lcd_handle);
    ~DHT11Sensor();

    esp_err_t start_task();
    void notify_read();
    float get_temperature();
    float get_humidity();
    void get_history(dht11_reading_t* history_buffer, uint32_t* num_readings);
    uint64_t get_last_read();
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t read_dht_data(float* temperature, float* humidity, bool suppressLogErrors);
void speaker_play_sound();

esp_err_t start_dht11_sensor_task(TaskHandle_t lcd_task_handle);
float dht11_get_temperature();
float dht11_get_humidity();
void dht11_notify_read();
void dht11_get_history(dht11_reading_t* history_buffer, uint32_t* num_readings);
uint64_t dht11_get_last_read();

#ifdef __cplusplus
}
#endif