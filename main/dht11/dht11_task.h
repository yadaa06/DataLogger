// dht11_task.h

#ifndef DHT11_TASK_H
#define DHT11_TASK_H


#define MAXATTEMPTS 3
#define MIN_READ_INTERVAL_US 3 * 1000 * 1000
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern SemaphoreHandle_t xDHT11Mutex;
extern TaskHandle_t dht11_task_handle;

void dht11_notify_read(void);
float dht11_get_temperature(void);
float dht11_get_humidity(void);
void dht11_read_task(void *pvParameters);

#endif