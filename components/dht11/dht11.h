// dht11.h

#ifndef DHT11_H
#define DHT11_H

#include "driver/gpio.h" 
#include "esp_err.h"     
#include <stdbool.h>

// DHT11 Pin Definition
#define DHT11_PIN GPIO_NUM_4

esp_err_t read_dht_data(float* temperature, float* humidity, bool suppressLogErrors);

#endif // DHT11_H