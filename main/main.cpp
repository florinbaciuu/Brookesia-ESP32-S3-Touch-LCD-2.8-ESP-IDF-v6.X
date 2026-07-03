

//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {

#include "esp_log.h"
#include "audio_pcm5101.h"
#include "lcd_backlight.h"
#include "lcd_bsp_interface.h"
#include "settings_nvs.h"
#include "touch_bsp_interface.h"
#include "lvgl_framework.h"
}

#include "brookesia_adapter.h"
#include "app_tabs_ui.h"
/**********************
 *   GLOBAL VARIABLES
 **********************/

//---------
//---------
//--------------------------------------

/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
*/
/*********************
 *  RTOS variables
 *********************/
// nothing here
//  -------------------------------
/********************************************** */
/*                   TASK                       */
/********************************************** */
// -------------------------------
// nothing here
/************************************************** */
// nothing here
//--------------------------------------

/*
███    ███  █████  ██ ███    ██ 
████  ████ ██   ██ ██ ████   ██ 
██ ████ ██ ███████ ██ ██ ██  ██ 
██  ██  ██ ██   ██ ██ ██  ██ ██ 
██      ██ ██   ██ ██ ██   ████ 
  * This is the main entry point of the application.
  * It initializes the hardware, sets up the display, and starts the LVGL tasks.
  * The application will run indefinitely until the device is powered off or reset.
*/
extern "C" void app_main(void) {
    static const char* TAG = "APP_MAIN";

    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "Starting application");

    settings_nvs_values_t saved_settings = {};
    const bool has_saved_settings = settings_nvs_load(&saved_settings) == ESP_OK;
    if (has_saved_settings) {
        (void)audio_pcm5101_set_volume(saved_settings.volume_percent);
    }

    Backlight_Driver_Init();  // Initialize the LCD backlight with default brightness 80%
    Backlight_Set(has_saved_settings ? saved_settings.brightness_percent : 100);
    lvgl_framework_init();    // Initialize the LVGL framework and display
    bsp_lcd_init();           // Initialize the LCD display
    bsp_touchscreen_init();   // Initialize the touch controller
    lvgl_kernel_start();      // Start the LVGL kernel and tasks
    if (!brookesia_adapter_start()) {
        lvgl_execute_locked(create_tabs_ui);
    }

    ESP_LOGI(TAG, "Application initialized");
}
