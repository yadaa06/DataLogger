// dht11_task.h

#ifndef DHT11_TASK_H
#define DHT11_TASK_H

#define MAXATTEMPTS 3
#define MIN_READ_INTERVAL_US 3000000
#define DHT_HISTORY_SIZE 60

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <time.h>

extern SemaphoreHandle_t xDHT11Mutex;
extern TaskHandle_t dht11_task_handle;
extern TaskHandle_t lcd_task_handle;

typedef struct {
    float temperature;
    float humidity;
    time_t timestamp;
} dht11_reading_t;

void dht11_notify_read(void);
float dht11_get_temperature(void);
float dht11_get_humidity(void);
void dht11_read_task(void* pvParameters);
void dht11_get_history(dht11_reading_t* history_buffer, uint32_t* num_readings);
uint64_t dht11_get_last_read(void);

#endif