#pragma once

#include <stdbool.h>
#include <stdint.h>

void bsp_touchscreen_init(void);

bool touch_read(uint16_t* x, uint16_t* y);