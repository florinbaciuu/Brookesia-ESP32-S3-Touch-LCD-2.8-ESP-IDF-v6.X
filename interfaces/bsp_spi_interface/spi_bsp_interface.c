
#include "spi_bsp_interface.h"
#include "board_config.h"
#include "board_pins.h"

#include "driver/spi_master.h"
#include "esp_log.h"

//======================================================================================================//

static const char* TAG = "SPI Interface";

//======================================================================================================//
/**
 * @brief spi master initialization
 */
void bsp_spi_init(void) {
    ESP_LOGI(TAG, "Initialize SPI bus");
    static bool initialized = false;
    if (initialized)
        return;
    spi_bus_config_t buscfg = {
        .sclk_io_num     = EXAMPLE_PIN_NUM_SCLK,
        .mosi_io_num     = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num     = EXAMPLE_PIN_NUM_MISO,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
    initialized = true;
    ESP_LOGI(TAG, "SPI bus initialized");
}
