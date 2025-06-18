// dht11.h

#ifndef DHT11_H
#define DHT11_H

#include "esp_err.h"   // For esp_err_t
#include "driver/gpio.h" // For GPIO_NUM_X

// DHT11 Pin Definition
#define DHT11_PIN                   GPIO_NUM_4
#define DHT11_POWER_ON_DELAY_MS     1000 

// Function Prototypes
/**
 * @brief Reads temperature and humidity data from the DHT11 sensor.
 *
 * @param temperature Pointer to a float where temperature will be stored.
 * @param humidity Pointer to a float where humidity will be stored.
 * @return esp_err_t ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t read_dht_data(float *temperature, float *humidity);

#endif // DHT11_H