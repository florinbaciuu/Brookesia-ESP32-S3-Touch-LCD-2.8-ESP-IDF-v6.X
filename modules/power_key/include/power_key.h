#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool latch_enabled;
    bool key_pressed;
} power_key_status_t;

esp_err_t power_key_init(void);
esp_err_t power_key_get_status(power_key_status_t* status);
esp_err_t power_key_set_latch(bool enabled);
bool power_key_is_ready(void);

#ifdef __cplusplus
}
#endif
