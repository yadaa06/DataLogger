// webserver.c

#include <stdio.h>
#include "webserver.h"
#include "dht11_task.h"

#include "esp_http_server.h"
#include "esp_err.h"
#include "esp_log.h"

static const char* TAG = "WEBSERVER_DRIVER";

static esp_err_t _root_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving /");
    float currentTemp = dht11_get_temperature();
    float currentHum = dht11_get_humidity(); 

    char resp_str[100];
    snprintf(resp_str, sizeof(resp_str), "Temperature: %.1f F, Humidity: %.1f %%", currentTemp, currentHum);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

httpd_uri_t root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = _root_get_handler,
    .user_ctx = NULL
};

httpd_handle_t start_webserver() {
    esp_err_t ret;
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting HTTP Server");
    ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error starting HTTP server");
        return NULL;
    }

    ESP_LOGI(TAG, "Registering URI handlers");
    ret = httpd_register_uri_handler(server, &root_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registering URI handlers");
        return NULL;
    }
    
    return server;
}
