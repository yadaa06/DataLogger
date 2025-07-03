// button.h

#ifndef BUTTON_H
#define BUTTON_H

#define BUTTON_GPIO GPIO_NUM_18

void button_task_init(void);
void button_press_task(void* pvParameters);


#endif