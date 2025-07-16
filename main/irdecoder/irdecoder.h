// irdecoder.h

#ifndef IR_DECODER_H
#define IR_DECODER_H

#define IR_PIN GPIO_NUM_14
#define IR_TIMES_SIZE 128


void ir_decoder_task(void* pvParameters);


#endif