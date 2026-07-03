#include "touch_cst328_driver.h"
#include "esp_log.h"
#include "touch_bsp_interface.h"
#include "i2c_bsp_interface.h"
#include "esp_lcd_touch.h"
#include "board_pins.h"
#include "board_config.h"

static const char* TAG = "Touchscreen BSP";

extern esp_lcd_touch_handle_t tp;

void bsp_touchscreen_init(void) {
    bsp_i2c_init();  // from i2c_bsp_interface, inițializează I2C pentru touch
    esp_lcd_panel_io_handle_t     tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST328_CONFIG();
    tp_io_config.scl_speed_hz                  = I2C_MASTER_FREQ_HZ;  // 🔥 FIX IMPORTANT
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_config, &tp_io_handle));

    esp_lcd_touch_config_t tp_cfg = {
        .x_max        = EXAMPLE_LCD_V_RES,
        .y_max        = EXAMPLE_LCD_H_RES,
        .rst_gpio_num = I2C_Touch_RST_IO,
        .int_gpio_num = I2C_Touch_INT_IO,
    };

    ESP_ERROR_CHECK(
        esp_lcd_touch_new_i2c_cst328(tp_io_handle, &tp_cfg, &tp));
    ESP_LOGI(TAG, "CST328 touch initializat corect");
}

bool touch_read(uint16_t* x, uint16_t* y) {
    uint16_t xs[1], ys[1], strength[1];
    uint8_t  points = 0;
    esp_lcd_touch_read_data(tp);
    bool touched = esp_lcd_touch_get_coordinates(
        tp,
        xs,
        ys,
        strength,
        &points,
        1);
    if (touched && points > 0) {
        *x = xs[0];
        *y = ys[0];
        return true;
    }
    return false;
}
