

#include <stdio.h>

#include "misc/lv_types.h"
#include "esp_lcd_types.h"


bool panel_io_trans_done_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx);

extern lv_display_t* disp;  
extern esp_lcd_panel_handle_t panel_handle;

void bsp_lcd_init(void);