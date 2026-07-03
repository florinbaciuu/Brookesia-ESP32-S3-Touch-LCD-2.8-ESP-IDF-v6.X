#include "aux_i2c_bsp_interface.h"

#include "board_config.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static const char* TAG = "AUX I2C Interface";

i2c_master_bus_handle_t aux_i2c_bus = NULL;

void bsp_aux_i2c_init(void)
{
    if (aux_i2c_bus != NULL) {
        return;
    }

    const i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_AUX_MASTER_NUM,
        .sda_io_num = I2C_AUX_SDA_IO,
        .scl_io_num = I2C_AUX_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &aux_i2c_bus));
    ESP_LOGI(TAG, "Auxiliary I2C bus ready on SDA=%d SCL=%d port=%d",
        I2C_AUX_SDA_IO,
        I2C_AUX_SCL_IO,
        I2C_AUX_MASTER_NUM);
}
