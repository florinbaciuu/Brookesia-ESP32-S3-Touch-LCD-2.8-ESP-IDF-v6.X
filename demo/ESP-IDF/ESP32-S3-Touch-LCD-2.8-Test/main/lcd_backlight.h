
#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"

extern uint8_t LCD_Backlight;

void Backlight_Init(void);                             // Initialize the LCD backlight, which has been called in the LCD_Init function, ignore it                                                         
void Set_Backlight(uint8_t Light);                   // Call this function to adjust the brightness of the backlight. The value of the parameter Light ranges from 0 to 100