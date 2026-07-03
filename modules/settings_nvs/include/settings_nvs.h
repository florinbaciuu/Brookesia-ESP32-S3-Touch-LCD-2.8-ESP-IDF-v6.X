#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t brightness_percent;
    uint8_t volume_percent;
    bool power_latch_enabled;
    bool navigation_bar_visible;
} settings_nvs_values_t;

esp_err_t settings_nvs_init(void);
esp_err_t settings_nvs_load(settings_nvs_values_t* values);
esp_err_t settings_nvs_save(const settings_nvs_values_t* values);
esp_err_t settings_nvs_set_brightness(uint8_t brightness_percent);
esp_err_t settings_nvs_set_volume(uint8_t volume_percent);
esp_err_t settings_nvs_set_power_latch(bool enabled);
esp_err_t settings_nvs_set_navigation_bar(bool visible);

#ifdef __cplusplus
}
#endif
