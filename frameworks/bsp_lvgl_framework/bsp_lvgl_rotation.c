
#include "esp_log.h"
#include "lvgl_framework.h"
#include "lvgl_framework_internals.h"
#include "display/lv_display.h"
#include "misc/lv_area.h"
#include "lcd_bsp_interface.h"
#include "esp_lcd_panel_ops.h"
// -------------------------------
// -------------------------------

/***
 * Rotation in runtime
 */
void lvgl_display_rotation_update_callback(lv_display_t* disp) {
    switch (lv_display_get_rotation(disp)) {
        case LV_DISPLAY_ROTATION_0:
            esp_lcd_panel_swap_xy(panel_handle, false);
            esp_lcd_panel_mirror(panel_handle, true, false);
            break;
        case LV_DISPLAY_ROTATION_90:
            esp_lcd_panel_swap_xy(panel_handle, true);
            esp_lcd_panel_mirror(panel_handle, true, true);
            break;
        case LV_DISPLAY_ROTATION_180:
            esp_lcd_panel_swap_xy(panel_handle, false);
            esp_lcd_panel_mirror(panel_handle, false, true);
            break;
        case LV_DISPLAY_ROTATION_270:
            esp_lcd_panel_swap_xy(panel_handle, true);
            esp_lcd_panel_mirror(panel_handle, false, false);
            break;
    }
}

// -------------------------------

void s_lvgl_display_panel_set_initial_rotation_config() {
#if (LVGL_DISPLAY_PANEL_ROTATION == DISPLAY_ROTATION_0)
    ESP_LOGI("LVGL", "Set display rotation to 0 degrees");
    lv_display_set_rotation(disp, (lv_display_rotation_t) LV_DISPLAY_ROTATION_0);  // Seteaza rotatia lvgl LV_DISPLAY_ROTATION_0
    lvgl_display_rotation_update_callback(disp);
#elif (LVGL_DISPLAY_PANEL_ROTATION == DISPLAY_ROTATION_90)
    ESP_LOGI("LVGL", "Set display rotation to 90 degrees");
    lv_display_set_rotation(disp, (lv_display_rotation_t) LV_DISPLAY_ROTATION_90);  // Seteaza rotatia lvgl LV_DISPLAY_ROTATION_90
    lvgl_display_rotation_update_callback(disp);
#elif (LVGL_DISPLAY_PANEL_ROTATION == DISPLAY_ROTATION_180)
    ESP_LOGI("LVGL", "Set display rotation to 180 degrees");
    lv_display_set_rotation(disp, (lv_display_rotation_t) LV_DISPLAY_ROTATION_180);  // Seteaza rotatia lvgl LV_DISPLAY_ROTATION_180
    lvgl_display_rotation_update_callback(disp);
#elif (LVGL_DISPLAY_PANEL_ROTATION == DISPLAY_ROTATION_270)
    ESP_LOGI("LVGL", "Set display rotation to 270 degrees");
    lv_display_set_rotation(disp, (lv_display_rotation_t) LV_DISPLAY_ROTATION_270);  // Seteaza rotatia lvgl LV_DISPLAY_ROTATION_270
    lvgl_display_rotation_update_callback(disp);
#endif  // LVGL_DISPLAY_PANEL_ROTATION
}

// -------------------------------

void lvgl_displ_rotate_now() {
    lvgl_display_rotation_update_callback(disp);
}
