#pragma once

#include "driver/i2c_master.h"

extern i2c_master_bus_handle_t i2c_bus;

void bsp_i2c_init(void);
