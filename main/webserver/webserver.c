// webserver.c

#include "webserver.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include <freertos/task.h>
#include "dht11_task.h"

static const char* TAG = "WEB_SERVER";
extern const uint8_t _binary_index_html_start[] asm("_binary_index_html_start");
extern const uint8_t _binary_index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t _binary_style_css_start[]  asm("_binary_style_css_start");
extern const uint8_t _binary_style_css_end[]    asm("_binary_style_css_end");
extern const uint8_t _binary_script_js_start[]  asm("_binary_script_js_start");
extern const uint8_t _binary_script_js_end[]    asm("_binary_script_js_end");

static esp_err_t _dht_history_get_handler(httpd_req_t *req) {
    dht11_reading_t *history_buffer = malloc(sizeof(dht11_reading_t) * DHT_HISTORY_SIZE);
    char *json_response = malloc(2048);

    if (history_buffer == NULL || json_response == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for history handler!");
        if (history_buffer) {
            free(history_buffer);
        }
        if (json_response) { 
            free(json_response);
        }
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    uint32_t number_of_readings = 0;
    dht11_get_history(history_buffer, &number_of_readings);

    char* p = json_response;
    const char* end = json_response + 2048;
    int len;

    ESP_LOGI(TAG, "Fetching %ld history readings.", number_of_readings);

    len = snprintf(p, end - p, "{\"history\":[");
    if (len < 0 || len >= (end - p)) {
        ESP_LOGE(TAG, "snprintf failed during JSON creation");
        goto cleanup; 
    }
    p += len;

    for (int i = 0; i < number_of_readings; i++) {
        len = snprintf(p, end - p, "{\"temperature\":%.2f,\"humidity\":%.1f,\"timestamp\":%lld}",
               history_buffer[i].temperature,
               history_buffer[i].humidity,
               (long long)history_buffer[i].timestamp);

        if (len < 0 || len >= (end - p)) { 
            goto cleanup; 
        }
        p += len;

        if (i < number_of_readings - 1) {
            if ((end - p) < 2) { 
                goto cleanup; 
            }
            *p++ = ',';
        }
    }

    if ((end - p) < 3) { 
        goto cleanup; 
    }
    *p++ = ']';
    *p++ = '}';
    *p = '\0';

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, strlen(json_response));
    ESP_LOGI(TAG, "Sent history data.");

    cleanup:
    free(history_buffer);
    free(json_response);

    return ESP_OK;
}


static esp_err_t _dht_data_get_handler(httpd_req_t *req) { 
    dht11_notify_read();

    vTaskDelay(pdMS_TO_TICKS(500));

    float temperature = dht11_get_temperature();
    float humidity = dht11_get_humidity();

    char json_response[64];
    int len = snprintf(json_response, sizeof(json_response), "{\"temperature\": %.2f, \"humidity\": %.1f}", temperature, humidity);

    if (len < 0 || len >= sizeof(json_response)) {
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

static esp_err_t _style_css_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving style.css");

    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char*)_binary_style_css_start, _binary_style_css_end - _binary_style_css_start);

    return ESP_OK;
}

static esp_err_t _script_js_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving script.js");

    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char*)_binary_script_js_start, _binary_script_js_end - _binary_script_js_start);

    return ESP_OK;
}

httpd_uri_t root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = _root_get_handler,
};

httpd_uri_t style_css_uri = {
    .uri = "/style.css",
    .method = HTTP_GET,
    .handler = _style_css_get_handler,
};

httpd_uri_t script_js_uri = {
    .uri = "/script.js",
    .method = HTTP_GET,
    .handler = _script_js_get_handler,
};

httpd_uri_t dht_data_uri = {
    .uri = "/dht_data",
    .method = HTTP_GET,
    .handler = _dht_data_get_handler,
};

httpd_uri_t dht_history_uri = {
    .uri = "/dht_history",
    .method = HTTP_GET,
    .handler = _dht_history_get_handler,
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

    ESP_LOGI(TAG, "Registering URI handler");
    ret = httpd_register_uri_handler(server, &root_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registering root URI handler");
        return NULL;
    }

    ret = httpd_register_uri_handler(server, &style_css_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registering style css handlers");
        return NULL;
    }

    ret = httpd_register_uri_handler(server, &script_js_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registering script_js handler");
        return NULL;
    }

    ret = httpd_register_uri_handler(server, &dht_data_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registering DHT data URI handler"); 
        return NULL;
    }

    ret = httpd_register_uri_handler(server, &dht_history_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registering DHT history URI handler"); 
        return NULL;
    } 
    
    return server;
}
