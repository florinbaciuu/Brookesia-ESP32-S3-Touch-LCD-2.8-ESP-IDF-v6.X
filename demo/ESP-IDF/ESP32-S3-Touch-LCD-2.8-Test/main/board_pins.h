#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_HOST  SPI3_HOST

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (80 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_MISO           46
#define EXAMPLE_PIN_NUM_MOSI           45
#define EXAMPLE_PIN_NUM_SCLK           40
#define EXAMPLE_PIN_NUM_LCD_CS         42
#define EXAMPLE_PIN_NUM_LCD_DC         41
#define EXAMPLE_PIN_NUM_LCD_RST        39
#define EXAMPLE_PIN_NUM_BK_LIGHT       5
// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES              240
#define EXAMPLE_LCD_V_RES              320
#define LCD_WIDTH                     EXAMPLE_LCD_H_RES
#define LCD_HEIGHT                    EXAMPLE_LCD_V_RES
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

#define Offset_X 0
#define Offset_Y 0


#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_HS_CH0_GPIO       EXAMPLE_PIN_NUM_BK_LIGHT
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_TEST_DUTY         (4000)
#define LEDC_ResolutionRatio   LEDC_TIMER_13_BIT
#define LEDC_MAX_Duty          ((1 << LEDC_ResolutionRatio) - 1)
#define Backlight_MAX   100      



#ifdef __cplusplus
}
#endif