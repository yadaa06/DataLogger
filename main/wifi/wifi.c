// wifi.c

#include "wifi.h"
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

static uint8_t retry_num = 0;
static const char* TAG = "WIFI_DRIVER";
static EventGroupHandle_t wifi_event_group;

static esp_err_t _wifi_driver_init(void) {
    ESP_LOGI(TAG, "Initializing Wifi driver");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition corrupted or not formatted. Erasing and re-initializing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    return ESP_OK;
}

static esp_err_t _wifi_driver_configure_station(void) {
    retry_num = 0;

    ESP_LOGI(TAG, "Configuring Wifi in Station Mode");
    ESP_ERROR_CHECK(esp_netif_init());

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    if (sta_netif == NULL) {
        ESP_LOGE(TAG, "Failed to create default WiFi station netif");
        return ESP_FAIL;
    }

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    return ESP_OK;
}

static esp_err_t _wifi_driver_connect_station(const char* ssid, const char* pswd) {
    ESP_LOGI(TAG, "Connecting to Wifi Network (%s)", ssid);

    wifi_config_t config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        }
    };

    strncpy((char*)config.sta.ssid, ssid, sizeof(config.sta.ssid));
    config.sta.ssid[sizeof(config.sta.ssid) - 1] = '\0';

    strncpy((char*)config.sta.password, pswd, sizeof(config.sta.password));
    config.sta.password[sizeof(config.sta.password) - 1] = '\0';

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config));
    ESP_ERROR_CHECK(esp_wifi_start()); 
    ESP_ERROR_CHECK(esp_wifi_connect());
 
    return ESP_OK;
}

static void _wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_START: WiFi station started. Attempting to connect...");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED: Connected to AP. Getting IP address...");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGW(TAG, "WIFI_EVENT_STA_DISCONNECTED: Disconnected from AP. Reason: %d",
                         ((wifi_event_sta_disconnected_t*)event_data)->reason);
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
                if (retry_num < MAX_RETRY) {
                    ESP_LOGI(TAG, "Retrying to connect to the AP... (%d/%d)", retry_num, MAX_RETRY);
                    retry_num++;
                    esp_wifi_connect();
                }
                else {
                    ESP_LOGE(TAG, "Max Wi-Fi retry attempts (%d) reached. Connection failed.", MAX_RETRY);
                    xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
                    xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                }
                break;
            default:
                ESP_LOGD(TAG, "Unhandled WIFI_EVENT: %s, ID: %d", event_base, (int)event_id);
                break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP: Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
                retry_num = 0;
                xEventGroupClearBits(wifi_event_group, WIFI_FAIL_BIT);
                xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
                break;
            }
            default:
                ESP_LOGD(TAG, "Unhandled IP_EVENT: %s, ID: %d", event_base, (int)event_id);
                break;
        }
    }
}

static esp_err_t _wifi_driver_event_handling() {
    esp_err_t ret;

    ESP_LOGI(TAG, "Starting Wifi Event Handling");

    wifi_event_group = xEventGroupCreate();

    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create WiFi Group");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "WiFi event group created");

    ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_wifi_event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register WIFI_EVENT handler (%s)", esp_err_to_name(ret));
        vEventGroupDelete(wifi_event_group);
        return ret;
    }
    ESP_LOGI(TAG, "Registered handler for all WIFI_EVENTs.");

    ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &_wifi_event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register IP_EVENT_STA_GOT_IP handler (%s)", esp_err_to_name(ret));
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &_wifi_event_handler);
        vEventGroupDelete(wifi_event_group);
        return ret;
    }
    ESP_LOGI(TAG, "Registered handler for IP_EVENT_STA_GOT_IP.");

    ESP_LOGI(TAG, "WiFi event handling setup complete.");
    return ESP_OK;
}

esp_err_t wifi_driver_start_and_connect(const char* ssid, const char* pswd) {
    ESP_ERROR_CHECK(_wifi_driver_init());
    ESP_ERROR_CHECK(_wifi_driver_configure_station());
    ESP_ERROR_CHECK(_wifi_driver_event_handling());
    ESP_ERROR_CHECK(_wifi_driver_connect_station(ssid, pswd));

    return ESP_OK;
}


EventGroupHandle_t wifi_driver_get_event_group() {
    return wifi_event_group;
}