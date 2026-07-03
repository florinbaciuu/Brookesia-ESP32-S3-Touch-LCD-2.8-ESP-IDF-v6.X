#include "touch_port.h"
#include "esp_lcd_touch.h"
#include "esp_log.h"

extern esp_lcd_touch_handle_t tp;

bool touch_read(uint16_t* x, uint16_t* y)
{
    uint16_t xs[1], ys[1], strength[1];
    uint8_t points = 0;

    esp_lcd_touch_read_data(tp);

    bool touched = esp_lcd_touch_get_coordinates(
        tp,
        xs,
        ys,
        strength,
        &points,
        1
    );

    if (touched && points > 0) {
        *x = xs[0];
        *y = ys[0];
        return true;
    }

    return false;
}
