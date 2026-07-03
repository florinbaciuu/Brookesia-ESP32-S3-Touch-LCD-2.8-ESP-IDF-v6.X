#pragma once

#include "driver/i2c_master.h"

extern i2c_master_bus_handle_t aux_i2c_bus;

void bsp_aux_i2c_init(void);
