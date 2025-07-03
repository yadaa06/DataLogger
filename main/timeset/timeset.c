// timeset.c

#include "timeset.h"
#include <time.h>
#include "esp_log.h"
#include "esp_sntp.h"

static const char* TAG = "TIMESET_DRIVER";

static void _initialize_sntp() {
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

static void _wait_for_time_sync() {
    time_t now = 0;
    struct tm timeinfo = {0};

    int retry = 0;

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < MAX_RETRY_COUNT) {
        ESP_LOGI(TAG, "Waiting for system time to be set (%d/%d)", retry, MAX_RETRY_COUNT);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    time(&now);
    localtime_r(&now, &timeinfo);
}

static void _set_timezone() {
    setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
    tzset();
    ESP_LOGI(TAG, "Timezone set to EST");
}

void setup_time() {
    _set_timezone();
    _initialize_sntp();
    _wait_for_time_sync();
}