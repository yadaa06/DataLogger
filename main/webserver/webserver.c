// webserver.c

#include <stdio.h>
#include "webserver.h"
#include "dht11_task.h"

#include "esp_http_server.h"
#include "esp_err.h"
#include "esp_log.h"

static const char* TAG = "WEB_SERVER";
extern const uint8_t _binary_index_html_start[] asm("_binary_index_html_start");
extern const uint8_t _binary_index_html_end[]   asm("_binary_index_html_end");

static esp_err_t _dht_data_get_handler(httpd_req_t *req) {
    float temperature = dht11_get_temperature();
    float humidity = dht11_get_humidity();

    char json_response[64];
    int len = snprintf(json_response, sizeof(json_response), "{\"temperature\": %.2f, \"humidity\": %.1f}", temperature, humidity);

    if (len < 0 || len > sizeof(json_response)) {
        ESP_LOGE(TAG, "JSON response buffer too small or snprintf error!");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to format JSON data");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, len);
    ESP_LOGI(TAG, "Sent DHT data: %s", json_response);
    return ESP_OK;
}



static esp_err_t _root_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving root page (/)");

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char*)_binary_index_html_start, _binary_index_html_end - _binary_index_html_start);

    return ESP_OK;
}

httpd_uri_t root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = _root_get_handler,
    .user_ctx = NULL
};

httpd_uri_t dht_data_uri = {
    .uri = "/dht_data",
    .method = HTTP_GET,
    .handler = _dht_data_get_handler,
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
    ret = httpd_register_uri_handler(server, &dht_data_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registering DHT data URI handler"); // Log for this handler
        return NULL;
    }
    
    return server;
}
