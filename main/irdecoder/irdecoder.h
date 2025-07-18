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

#endif