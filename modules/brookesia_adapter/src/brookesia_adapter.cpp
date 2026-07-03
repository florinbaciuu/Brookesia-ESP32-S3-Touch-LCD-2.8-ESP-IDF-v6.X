#include "brookesia_adapter.h"
#include "brookesia_project_board.h"
#include "brookesia_diagnostics_ui.h"
#include "brookesia_phone_launcher.h"

extern "C" {
#include "esp_log.h"
#include "lvgl_framework.h"
}

#include "brookesia/hal_interface.hpp"

static const char* TAG = "BROOKESIA_ADAPTER";

#if CONFIG_BROOKESIA_ADAPTER_ENABLE && CONFIG_BROOKESIA_ADAPTER_LOCAL_LVGL_SHELL
static void create_diagnostics_shell(void) {
    brookesia_diagnostics_ui_create(brookesia_project_board_get_status);
}

static bool s_phone_launcher_started = false;

static void create_phone_launcher(void) {
    s_phone_launcher_started = brookesia_phone_launcher_start();
}
#endif

bool brookesia_adapter_start(void) {
#if CONFIG_BROOKESIA_ADAPTER_ENABLE
    ESP_LOGI(TAG, "Starting Brookesia adapter");

#if CONFIG_BROOKESIA_ADAPTER_LOCAL_LVGL_SHELL
    brookesia_storage_automount_start();

    if (!brookesia_project_board_init()) {
        ESP_LOGE(TAG, "Brookesia project board HAL init failed");
        return false;
    }

#if CONFIG_BROOKESIA_ADAPTER_PHONE_LAUNCHER_ENABLE
    s_phone_launcher_started = false;
    lvgl_execute_locked(create_phone_launcher);
    if (s_phone_launcher_started) {
        ESP_LOGI(TAG, "ESP-Brookesia Phone launcher started");
        return true;
    }
    ESP_LOGW(TAG, "ESP-Brookesia Phone launcher failed; trying local diagnostics shell");
#endif

#if CONFIG_BROOKESIA_ADAPTER_LOCAL_LVGL_SHELL
    lvgl_execute_locked(create_diagnostics_shell);
    ESP_LOGI(TAG, "Local LVGL Brookesia shell started");
    return true;
#else
    ESP_LOGE(TAG, "No Brookesia UI backend started");
    return false;
#endif
#else
    ESP_LOGW(TAG, "Brookesia adapter enabled, but no runtime backend is selected");
    return false;
#endif

#else
    ESP_LOGI(TAG, "Brookesia adapter disabled; keeping existing UI");
    return false;
#endif
}
