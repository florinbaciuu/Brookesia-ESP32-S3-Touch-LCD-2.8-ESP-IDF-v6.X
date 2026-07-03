
#include "i2c_bsp_interface.h"
#include "board_config.h"
#include "board_pins.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "touch_cst328_driver.h"

//======================================================================================================//

static const char* TAG          = "I2C Interface";

i2c_master_bus_handle_t i2c_bus = NULL;

//======================================================================================================//

/**
 * @brief i2c master initialization
 */
void bsp_i2c_init(void) {
    if (i2c_bus) {
        return;
    }

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port                     = I2C_Touch_MASTER_NUM,  // I2C_NUM_0 sau 1
        .sda_io_num                   = I2C_Touch_SDA_IO,
        .scl_io_num                   = I2C_Touch_SCL_IO,
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(
        i2c_new_master_bus(&bus_cfg, &i2c_bus));

    ESP_LOGI(TAG, "I2C master bus creat corect (driver i2c_master)");
}

//======================================================================================================//