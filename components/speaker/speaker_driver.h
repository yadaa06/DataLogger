// speaker_driver.h

#pragma once

#include <stdint.h>
#include <stddef.h>

#define SPEAKER_UL_VALUE 9 

void speaker_play_sound(void);
void speaker_driver_play_task(void* pvParameters);