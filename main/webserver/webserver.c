// webserver.c

#include <stdio.h>
#include "webserver.h"
#include "dht11_task.h"

#include "esp_http_server.h"
#include "esp_err.h"
#include "esp_log.h"

static const char* TAG = "WEB_SERVER";

const char* HTML_PAGE_CONTENT = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name = "viewport" content = "width=device-width, initial-scale = 1.0">
    <title> ESP32 DHT11 Monitor</title>
    
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, Sans-Serif;
            background-color: #1c1c1c;
            color: #9e9e9e;
            text-align: center;
            margin: 20px;
            padding: 0;
            line-height: 1.6;
        }
        .container {
            background-color: #303030;
            border-radius: 10px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            padding: 30px;
            max-width: 500px;
            margin: 50px auto;
        }
        p {
            font-size: 1.2em;
            margin: 10px 0;
            color: #8e8f8b;
            font-weight: bold;
        }
        .last-updated {
            font-size: 0.9em;
            color: #757575;
            margin-top: 20px;
        }
        button {
            background-color: #797979;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, Sans-Serif;
            font-weight: bold;
            border-radius: 10px;
            border: none;
            border-color: #030303;
            transition: background-color 0.3s ease;
            cursor: pointer;
            color: #030303;
        }
        button:hover{
            background-color: #7c7d94;
        }
    </style>
</head>
<body>
    <div class = "container">
        <h1> Current Readings</h1>
        <p>Temperature: <span id = "temperature">--.--</span> &deg;F</p>
        <p>Humidity: <span id = "humidity">--.-</span> %</p>
        <p class = "last-updated">Last Updated: <span id = "lastupdated">N/A</span></p>
    </div>
    <button id = "readNowButton">Get Current Reading Now</button>
</body>
    <script>
        async function updateDHTdata() {
            try {
                const response = await fetch('/dht_data');
                const data = await response.json();
                
                document.getElementById('temperature').textContent = data.temperature.toFixed(2);
                document.getElementById('humidity').textContent = data.humidity.toFixed(1);
                
                const now = new Date();
                document.getElementById('lastupdated').textContent = now.toLocaleTimeString();
                
                console.log("Data updated successfully:", data);
            } catch (error) {
                console.error("Error fetching DHT data:", error);
                document.getElementById('temperature').textContent = "Error";
                document.getElementById('humidity').textContent = "Error";
                document.getElementById('lastupdated').textContent = "Error";
            }
        }
        
        const readNowBtn = document.getElementById('readNowButton');
        if (readNowButton) {
            readNowBtn.addEventListener('click', () => {
                console.log("Read Now Button Clicked");
                updateDHTdata();
            });
        }
        
        document.addEventListener('DOMContentLoaded', updateDHTdata);
        setInterval(updateDHTdata, 60000);
    </script>
</html>
)rawliteral";

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

    httpd_resp_send(req, HTML_PAGE_CONTENT, HTTPD_RESP_USE_STRLEN);

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
