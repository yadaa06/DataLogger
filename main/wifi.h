// wifi.h

#ifndef WIFI_H
#define WIFI_H
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define WIFI_MAX_SSID_LEN       32
#define WIFI_MAX_PSWD_LEN       64
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1
#define MAX_RETRY               10

esp_err_t wifi_driver_init(void);
esp_err_t wifi_driver_configure_station(void);
esp_err_t wifi_driver_connect_station(const char* ssid, const char* pswd);
esp_err_t wifi_driver_event_handling(void);

EventGroupHandle_t wifi_driver_get_event_group(void);

#endif