// irdecoder.h

#pragma once

#include <stdint.h>

#define IR_PIN GPIO_NUM_14
#define IR_TIMES_SIZE 128

#define TIMEOUT_US 100000

#define NEC_START_PULSE 9000
#define NEC_START_SPACE 4500
#define NEC_BIT_PULSE 560
#define NEC_ZERO_SPACE 560
#define NEC_ONE_SPACE 1690
#define NEC_TOLERANCE_PERCENT 30

#define IR_MATCH(value, target)                                          \
    ((value) >= ((target) - ((target) * NEC_TOLERANCE_PERCENT / 100)) && \
     (value) <= ((target) + ((target) * NEC_TOLERANCE_PERCENT / 100)))

typedef enum {
    IR_FRAME_TYPE_DATA,
    IR_FRAME_TYPE_REPEAT,
    IR_FRAME_TYPE_INVALID,
} ir_frame_type_t;

typedef enum {
    BUTTON_0                = 0x68,
    BUTTON_1                = 0x30,
    BUTTON_2                = 0x18,
    BUTTON_3                = 0x7A,
    BUTTON_4                = 0x10,
    BUTTON_5                = 0x38,
    BUTTON_6                = 0x5A,
    BUTTON_7                = 0x42,
    BUTTON_8                = 0x4A,
    BUTTON_9                = 0x52,
    BUTTON_PLUS             = 0x90,
    BUTTON_MINUS            = 0xA8,
    BUTTON_EQ               = 0xE0,
    BUTTON_U_SD             = 0xB0,
    BUTTON_CYCLE            = 0x98,
    BUTTON_PLAY_PAUSE       = 0x22,
    BUTTON_BACKWARD         = 0x02,
    BUTTON_FORWARD          = 0xC2,
    BUTTON_POWER            = 0xA2,
    BUTTON_MUTE             = 0xE2,
    BUTTON_MODE             = 0x62,
    BUTTON_UNKNOWN_OR_ERROR = 0xFF
} button_press_t;

typedef struct {
    ir_frame_type_t type;
    button_press_t button;
    uint8_t address;
    uint8_t command;
} ir_result_t;

void ir_decode_task(void* pvParameters);