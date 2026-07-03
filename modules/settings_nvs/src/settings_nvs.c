#include "settings_nvs.h"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char* TAG = "SETTINGS_NVS";
static const char* NAMESPACE = "board_cfg";
static const char* KEY_BRIGHTNESS = "brightness";
static const char* KEY_VOLUME = "volume";
static const char* KEY_POWER_LATCH = "pwr_latch";
static const char* KEY_NAV_BAR = "nav_bar";

static bool s_initialized = false;
static settings_nvs_values_t s_values = {
    .brightness_percent = 100,
    .volume_percent = 50,
    .power_latch_enabled = true,
    .navigation_bar_visible = false,
};

static uint8_t clamp_percent(uint8_t value, uint8_t min_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > 100) {
        return 100;
    }
    return value;
}

static esp_err_t open_namespace(nvs_open_mode_t mode, nvs_handle_t* handle)
{
    esp_err_t err = settings_nvs_init();
    if (err != ESP_OK) {
        return err;
    }

    return nvs_open(NAMESPACE, mode, handle);
}

esp_err_t settings_nvs_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs erase: %s", esp_err_to_name(err));
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err == ESP_OK) {
        s_initialized = true;
        ESP_LOGI(TAG, "NVS ready");
    } else {
        ESP_LOGW(TAG, "NVS init failed: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t settings_nvs_load(settings_nvs_values_t* values)
{
    if (values == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle = 0;
    esp_err_t err = open_namespace(NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        *values = s_values;
        return ESP_OK;
    }
    if (err != ESP_OK) {
        return err;
    }

    uint8_t u8_value = 0;
    if (nvs_get_u8(handle, KEY_BRIGHTNESS, &u8_value) == ESP_OK) {
        s_values.brightness_percent = clamp_percent(u8_value, 5);
    }
    if (nvs_get_u8(handle, KEY_VOLUME, &u8_value) == ESP_OK) {
        s_values.volume_percent = clamp_percent(u8_value, 0);
    }
    if (nvs_get_u8(handle, KEY_POWER_LATCH, &u8_value) == ESP_OK) {
        s_values.power_latch_enabled = u8_value != 0;
    }
    if (nvs_get_u8(handle, KEY_NAV_BAR, &u8_value) == ESP_OK) {
        s_values.navigation_bar_visible = u8_value != 0;
    }

    nvs_close(handle);
    *values = s_values;
    return ESP_OK;
}

esp_err_t settings_nvs_save(const settings_nvs_values_t* values)
{
    if (values == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle = 0;
    esp_err_t err = open_namespace(NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    s_values = *values;
    s_values.brightness_percent = clamp_percent(s_values.brightness_percent, 5);
    s_values.volume_percent = clamp_percent(s_values.volume_percent, 0);

    err = nvs_set_u8(handle, KEY_BRIGHTNESS, s_values.brightness_percent);
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, KEY_VOLUME, s_values.volume_percent);
    }
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, KEY_POWER_LATCH, s_values.power_latch_enabled ? 1 : 0);
    }
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, KEY_NAV_BAR, s_values.navigation_bar_visible ? 1 : 0);
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

esp_err_t settings_nvs_set_brightness(uint8_t brightness_percent)
{
    settings_nvs_values_t values = {};
    esp_err_t err = settings_nvs_load(&values);
    if (err != ESP_OK) {
        return err;
    }

    values.brightness_percent = clamp_percent(brightness_percent, 5);
    return settings_nvs_save(&values);
}

esp_err_t settings_nvs_set_volume(uint8_t volume_percent)
{
    settings_nvs_values_t values = {};
    esp_err_t err = settings_nvs_load(&values);
    if (err != ESP_OK) {
        return err;
    }

    values.volume_percent = clamp_percent(volume_percent, 0);
    return settings_nvs_save(&values);
}

esp_err_t settings_nvs_set_power_latch(bool enabled)
{
    settings_nvs_values_t values = {};
    esp_err_t err = settings_nvs_load(&values);
    if (err != ESP_OK) {
        return err;
    }

    values.power_latch_enabled = enabled;
    return settings_nvs_save(&values);
}

esp_err_t settings_nvs_set_navigation_bar(bool visible)
{
    settings_nvs_values_t values = {};
    esp_err_t err = settings_nvs_load(&values);
    if (err != ESP_OK) {
        return err;
    }

    values.navigation_bar_visible = visible;
    return settings_nvs_save(&values);
}
