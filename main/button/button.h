// button.h

#ifndef BUTTON_H
#define BUTTON_H

#define BUTTON_GPIO GPIO_NUM_18
#define DEBOUNCE_TIME_MS 200

void button_press_task(void* pvParameters);


#endif