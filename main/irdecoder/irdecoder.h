// irdecoder.h

#ifndef IR_DECODER_H
#define IR_DECODER_H

#define IR_PIN GPIO_NUM_14
#define IR_TIMES_SIZE 128

#define NEC_START_PULSE 9000
#define NEC_START_SPACE 4500
#define NEC_BIT_PULSE 560 
#define NEC_ZERO_SPACE 560 
#define NEC_ONE_SPACE 1690 
#define NEC_TOLERANCE_PERCENT 30 

#define IR_MATCH(value, target) \
    ((value) >= ((target) - ((target) * NEC_TOLERANCE_PERCENT / 100)) && \
    (value) <= ((target) + ((target) * NEC_TOLERANCE_PERCENT / 100)))

void ir_decoder_task(void* pvParameters);

typedef enum {
    IR_FRAME_TYPE_DATA,
    IR_FRAME_TYPE_REPEAT,
    IR_FRAME_TYPE_INVALID,
} ir_frame_type_t;

typedef struct {
    ir_frame_type_t type;
    uint8_t address;
    uint8_t command;
} ir_result_t;

#endif