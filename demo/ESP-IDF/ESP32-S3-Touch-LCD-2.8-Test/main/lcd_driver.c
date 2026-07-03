

#include "lcd_driver.h"

static const char* TAG_LCD = "ST7789";

lv_display_t*          disp;
esp_lcd_panel_handle_t panel_handle = NULL;

void LCD_Init(void) {
    ESP_LOGI(TAG_LCD, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num     = EXAMPLE_PIN_NUM_SCLK,
        .mosi_io_num     = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num     = EXAMPLE_PIN_NUM_MISO,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG_LCD, "Install panel IO");
    esp_lcd_panel_io_handle_t     io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num         = EXAMPLE_PIN_NUM_LCD_DC,
        .cs_gpio_num         = EXAMPLE_PIN_NUM_LCD_CS,
        .pclk_hz             = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits        = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits      = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode            = 0,
        .trans_queue_depth   = 10,
        .on_color_trans_done = panel_io_trans_done_callback,
        .user_ctx            = disp,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_st7789t_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_endian     = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    ESP_LOGI(TAG_LCD, "Install ST7789 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789t(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));

    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG_LCD, "Turn on LCD backlight");
    // gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
}

bool panel_io_trans_done_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx) {
#ifdef flush_ready_in_io_trans_done
    lv_display_t* d = (lv_display_t*) user_ctx;
    if (d) {
        lv_disp_flush_ready(d);
        
    }
#endif /* #ifdef flush_ready_in_io_trans_done */
    return false;
}
