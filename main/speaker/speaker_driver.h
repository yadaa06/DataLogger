// speaker_driver.h

#ifndef SPEAKER_DRIVER_H
#define SPEAKER_DRIVER_H

#include <stdint.h>
#include <stddef.h>

#define SAMPLE_RATE 8000

void speaker_driver_init(void);
void speaker_driver_play(void);
void speaker_driver_deinit(void);
void speaker_driver_play_task(void* pvParameters);

#endif