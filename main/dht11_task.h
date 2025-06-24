// dht11_task.h

#ifndef DHT11_TASK_H
#define DHT11_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


float dht11_get_temperature(void);
float dht11_get_humidity(void);
void dht11_read_task(void *pvParameters);

#endif