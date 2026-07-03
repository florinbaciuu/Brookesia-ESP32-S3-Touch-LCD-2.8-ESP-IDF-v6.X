#include "brookesia_phone_launcher.h"

#include <inttypes.h>
#include <time.h>

extern "C" {
#include "esp_log.h"
#include "lvgl.h"
#include "lvgl_framework.h"
}

#include "esp_brookesia.hpp"
#include "phone_app_board_info.hpp"
#include "phone_app_sensors.hpp"

static const char* TAG = "BROOKESIA_PHONE";

static ESP_Brookesia_Phone* s_phone = nullptr;

static bool phone_lv_lock(int timeout_ms) {
    return lvgl_lock(timeout_ms);
}

static void phone_lv_unlock(void) {
    lvgl_unlock();
}

static void update_clock_timer_cb(lv_timer_t* timer) {
    ESP_Brookesia_Phone* phone = static_cast<ESP_Brookesia_Phone*>(lv_timer_get_user_data(timer));
    if (phone == nullptr) {
        return;
    }

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    phone->getHome().getStatusBar()->setClock(timeinfo.tm_hour, timeinfo.tm_min);
}

static bool apply_best_stylesheet(ESP_Brookesia_Phone* phone, int32_t width, int32_t height) {
    ESP_Brookesia_PhoneStylesheet_t stylesheet = ESP_BROOKESIA_PHONE_DEFAULT_DARK_STYLESHEET();

    if (width == 320 && height == 240) {
        stylesheet = ESP_BROOKESIA_PHONE_320_240_DARK_STYLESHEET();
    } else if (width == 320 && height == 480) {
        stylesheet = ESP_BROOKESIA_PHONE_320_480_DARK_STYLESHEET();
    } else if (width == 240 && height == 320) {
        stylesheet.home.app_launcher.data.table.default_num = 1;
        stylesheet.home.app_launcher.data.table.size = ESP_BROOKESIA_STYLE_SIZE_RECT_PERCENT(100, 76);
        stylesheet.home.app_launcher.data.icon.main.size = ESP_BROOKESIA_STYLE_SIZE_RECT(90, 96);
        stylesheet.home.app_launcher.data.icon.main.layout_row_pad = 4;
        stylesheet.home.app_launcher.data.icon.image.default_size = ESP_BROOKESIA_STYLE_SIZE_SQUARE(48);
        stylesheet.home.app_launcher.data.icon.image.press_size = ESP_BROOKESIA_STYLE_SIZE_SQUARE(42);
        stylesheet.home.app_launcher.data.icon.label.text_font = ESP_BROOKESIA_STYLE_FONT_SIZE(12);
        stylesheet.home.app_launcher.data.indicator.main_size = ESP_BROOKESIA_STYLE_SIZE_RECT_W_PERCENT(100, 14);
        stylesheet.home.app_launcher.data.indicator.main_layout_column_pad = 6;
        stylesheet.home.app_launcher.data.indicator.main_layout_bottom_offset = 8;
        stylesheet.home.app_launcher.data.indicator.spot_inactive_size = ESP_BROOKESIA_STYLE_SIZE_SQUARE(6);
        stylesheet.home.app_launcher.data.indicator.spot_active_size = ESP_BROOKESIA_STYLE_SIZE_RECT(18, 6);
        stylesheet.manager.flags.enable_gesture = 1;
        stylesheet.manager.flags.enable_gesture_navigation_back = 1;
        ESP_LOGI(TAG, "Using Waveshare 240x320 launcher grid: 2 columns x 2 rows");
    } else {
        ESP_LOGW(TAG, "No exact Phone stylesheet for %" PRId32 "x%" PRId32 "; using default adaptive style", width, height);
    }

    ESP_LOGI(TAG, "Using Phone stylesheet: %s", stylesheet.core.name);
    if (!phone->addStylesheet(stylesheet)) {
        ESP_LOGE(TAG, "Failed to add Phone stylesheet");
        return false;
    }
    if (!phone->activateStylesheet(stylesheet)) {
        ESP_LOGE(TAG, "Failed to activate Phone stylesheet");
        return false;
    }

    return true;
}

static void log_app_install_result(const char* app_name, int app_id) {
    if (app_id < 0) {
        ESP_LOGW(TAG, "Failed to install Phone app: %s", app_name);
        return;
    }

    ESP_LOGI(TAG, "Installed Phone app: %s id=%d", app_name, app_id);
}

bool brookesia_phone_launcher_start(void) {
#if CONFIG_BROOKESIA_ADAPTER_PHONE_LAUNCHER_ENABLE
    if (s_phone != nullptr) {
        ESP_LOGW(TAG, "Phone launcher already started");
        return true;
    }

    lv_display_t* display = lv_display_get_default();
    if (display == nullptr) {
        ESP_LOGE(TAG, "LVGL display is not available");
        return false;
    }

    lv_indev_t* touch = lv_indev_get_next(nullptr);
    if (touch == nullptr || lv_indev_get_type(touch) != LV_INDEV_TYPE_POINTER) {
        ESP_LOGE(TAG, "LVGL pointer input device is not available");
        return false;
    }

    ESP_Brookesia_Phone* phone = new ESP_Brookesia_Phone(display);
    if (phone == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate Phone launcher");
        return false;
    }

    const int32_t width = lv_display_get_horizontal_resolution(display);
    const int32_t height = lv_display_get_vertical_resolution(display);
    if (!apply_best_stylesheet(phone, width, height)) {
        delete phone;
        return false;
    }

    if (!phone->setTouchDevice(touch)) {
        ESP_LOGE(TAG, "Failed to attach touch device");
        delete phone;
        return false;
    }

    phone->registerLvLockCallback(phone_lv_lock, -1);
    phone->registerLvUnlockCallback(phone_lv_unlock);

    if (!phone->begin()) {
        ESP_LOGE(TAG, "Phone launcher begin failed");
        delete phone;
        return false;
    }

    PhoneAppBoardInfo* board_app = new PhoneAppBoardInfo();
    if (board_app == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate Phone app: Board");
    } else {
        log_app_install_result("Board", phone->installApp(board_app));
    }

    PhoneAppSensors* sensors_app = new PhoneAppSensors();
    if (sensors_app == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate Phone app: Sensors");
    } else {
        log_app_install_result("Sensors", phone->installApp(sensors_app));
    }

    lv_timer_create(update_clock_timer_cb, 1000, phone);
    s_phone = phone;

    ESP_LOGI(TAG, "ESP-Brookesia Phone launcher started");
    return true;
#else
    ESP_LOGI(TAG, "Phone launcher disabled");
    return false;
#endif
}
