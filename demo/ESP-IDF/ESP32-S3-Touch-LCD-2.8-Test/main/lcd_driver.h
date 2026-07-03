
#include "CONFIG.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "lvgl.h"


#include "Vernon_ST7789T.h"
#include "board_pins.h"

bool panel_io_trans_done_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx);

extern lv_display_t* disp;  
extern esp_lcd_panel_handle_t panel_handle;

void LCD_Init(void);