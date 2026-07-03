

//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd_backlight.h"
#include "lcd_driver.h"
#include "lcd_touch_driver.h"
#include "lvgl_port.h"
#include "ui.h"
#include "filesystem.h"
}
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

// -------------------------------
/********************************************** */
/*                   TASK                       */
/********************************************** */
// -------------------------------

/************************************************** */

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
    esp_log_level_set("*", ESP_LOG_INFO);  //
    printf("Starting application!\n");
    ESP_LOGI("APP_MAIN", "Hello world!\n");
    printf("Hello world!\n");

    Backlight_Init();   // Initialize the LCD backlight with default brightness 80%
    Set_Backlight(100);  // Set backlight brightness to 80%
    s_lvgl_port_init();
    LCD_Init();  // Initialize the LCD display
    TOUCH_Init();  // Initialize the touch controller

    create_and_start_lvgl_tasks();  // freetos tasks for lvgl

    s_lvgl_lock(portMAX_DELAY);
    create_tabs_ui();
    s_lvgl_unlock();

    init_filesystem_sys();

    vTaskDelay(pdMS_TO_TICKS(150));
    //fflush(stdout);
    printf("Closing application!\n");
}
