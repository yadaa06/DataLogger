// timeset.h

#pragma once

#include "esp_err.h"

enum {
    TIME_SYNC_COMPLETE,
};

esp_err_t timeset_driver_start_and_wait(void);