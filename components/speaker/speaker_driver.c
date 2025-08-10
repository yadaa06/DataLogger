// speaker_driver.c

#include "speaker_driver.h"
#include "audio_data.h"
#include "driver/dac_continuous.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

#define I2S_PORT I2S_NUM_0

static const char* TAG = "AUDIO_DRIVER";
static TaskHandle_t speaker_task_handle = NULL;
dac_continuous_handle_t dac_handle;

static void speaker_driver_init(void) {
    dac_continuous_config_t dac_cfg = {
        .chan_mask = DAC_CHANNEL_MASK_CH0,
        .desc_num = 4,
        .buf_size = 1024,
        .freq_hz = 16000,
        .offset = 0,
        .clk_src = DAC_DIGI_CLK_SRC_APLL,
    };

    ESP_ERROR_CHECK(dac_continuous_new_channels(&dac_cfg, &dac_handle));
    ESP_ERROR_CHECK(dac_continuous_enable(dac_handle));
}

static void speaker_driver_play(void) {
    size_t written = 0;
    ESP_ERROR_CHECK(dac_continuous_write(dac_handle, audio_fx, audio_fx_len, &written, portMAX_DELAY));
}

static void speaker_driver_deinit(void) {
    dac_continuous_disable(dac_handle);
    dac_continuous_del_channels(dac_handle);
}

void speaker_play_sound(void) {
    if (speaker_task_handle != NULL) {
        xTaskNotify(speaker_task_handle, SPEAKER_UL_VALUE, eSetValueWithoutOverwrite);
    }
}

void speaker_driver_play_task(void* pvParameters) {
    (void)pvParameters;
    ESP_LOGI(TAG, "Starting Speaker Task");
    speaker_task_handle = xTaskGetCurrentTaskHandle();

    while (1) {
        uint32_t ulNotifiedValue;
        BaseType_t xResult = xTaskNotifyWait(0, UINT32_MAX, &ulNotifiedValue, pdMS_TO_TICKS(30000));

        if (xResult == pdTRUE && ulNotifiedValue == SPEAKER_UL_VALUE) {
            ESP_LOGI(TAG, "Sound triggered");
        }

        speaker_driver_init();
        speaker_driver_play();
        ESP_LOGI(TAG, "SOUND PLAYED");
        speaker_driver_deinit();
    }
}