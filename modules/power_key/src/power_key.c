#include "power_key.h"

#include "board_config.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "POWER_KEY";

static bool s_initialized = false;
static bool s_latch_enabled = false;

esp_err_t power_key_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    gpio_config_t input_cfg = {
        .pin_bit_mask = 1ULL << PWR_KEY_INPUT_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&input_cfg);
    if (err != ESP_OK) {
        return err;
    }

    gpio_config_t latch_cfg = {
        .pin_bit_mask = 1ULL << PWR_CONTROL_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    err = gpio_config(&latch_cfg);
    if (err != ESP_OK) {
        return err;
    }

    err = power_key_set_latch(true);
    if (err != ESP_OK) {
        return err;
    }

    s_initialized = true;
    ESP_LOGI(TAG,
        "Power key ready on input GPIO%d, latch GPIO%d",
        PWR_KEY_INPUT_GPIO,
        PWR_CONTROL_GPIO);
    return ESP_OK;
}

esp_err_t power_key_get_status(power_key_status_t* status)
{
    if (status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_err_t init_err = power_key_init();
    if (init_err != ESP_OK) {
        return init_err;
    }

    status->latch_enabled = s_latch_enabled;
    status->key_pressed = gpio_get_level(PWR_KEY_INPUT_GPIO) == 0;
    return ESP_OK;
}

esp_err_t power_key_set_latch(bool enabled)
{
    const esp_err_t err = gpio_set_level(PWR_CONTROL_GPIO, enabled ? 1 : 0);
    if (err == ESP_OK) {
        s_latch_enabled = enabled;
    }
    return err;
}

bool power_key_is_ready(void)
{
    return s_initialized;
}
