


#pragma once


#include "esp_lcd_touch.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"


#ifdef __cplusplus
extern "C" {
#endif

extern esp_lcd_touch_handle_t tp;

esp_err_t esp_lcd_touch_new_i2c_cst328(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *out_touch);

void Touch_Driver_Init(void);

#ifdef __cplusplus
}
#endif
