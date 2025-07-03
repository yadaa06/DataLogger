// lcd_task.h

#ifndef LCD_TASK_H
#define LCD_TASK_H

#define BUTTON_UL_VALUE 14

typedef enum {
    LCD_MODE_TEMP,
    LCD_MODE_HUM,
    LCD_MODE_LAST_READ,
    LCD_MODE_MAX
} lcd_mode_t;

void lcd_display_task(void *pvParameters);
void lcd_cycle_mode(void);

#endif