#include "phone_app_board_info.hpp"

extern "C" {
#include "lvgl.h"
#include "sdkconfig.h"
}

namespace {

ESP_Brookesia_CoreAppData_t create_board_core_data()
{
    return ESP_BROOKESIA_CORE_APP_DATA_DEFAULT("Board", nullptr, true);
}

ESP_Brookesia_PhoneAppData_t create_board_phone_data()
{
    ESP_Brookesia_PhoneAppData_t data = ESP_BROOKESIA_PHONE_APP_DATA_DEFAULT(nullptr, true, true);
    data.flags.enable_navigation_gesture = 0;
    return data;
}

lv_obj_t* create_label(lv_obj_t* parent, const char* text, bool heading = false)
{
    lv_obj_t* label = lv_label_create(parent);
    if (label == nullptr) {
        return nullptr;
    }

    lv_label_set_text(label, text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_color(label, lv_color_hex(heading ? 0xFFFFFF : 0xD6DEE8), 0);
    lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
    lv_obj_set_style_pad_bottom(label, heading ? 6 : 2, 0);
    return label;
}

void create_info_row(lv_obj_t* parent, const char* title, const char* value)
{
    lv_obj_t* row = lv_obj_create(parent);
    if (row == nullptr) {
        return;
    }

    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_bottom(row, 5, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(row, LV_OBJ_FLAG_EVENT_BUBBLE | LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_t* title_label = lv_label_create(row);
    if (title_label != nullptr) {
        lv_label_set_text(title_label, title);
        lv_obj_set_width(title_label, lv_pct(100));
        lv_obj_set_style_text_color(title_label, lv_color_hex(0x8DA2B8), 0);
        lv_obj_set_style_text_font(title_label, LV_FONT_DEFAULT, 0);
    }

    lv_obj_t* value_label = lv_label_create(row);
    if (value_label != nullptr) {
        lv_label_set_text(value_label, value);
        lv_label_set_long_mode(value_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(value_label, lv_pct(100));
        lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(value_label, LV_FONT_DEFAULT, 0);
    }
}

} // namespace

PhoneAppBoardInfo::PhoneAppBoardInfo()
    : ESP_Brookesia_PhoneApp(create_board_core_data(), create_board_phone_data())
{
}

bool PhoneAppBoardInfo::run(void)
{
    lv_obj_t* screen = lv_scr_act();
    if (screen == nullptr) {
        return false;
    }

    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t* root = lv_obj_create(screen);
    if (root == nullptr) {
        return false;
    }

    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_left(root, 10, 0);
    lv_obj_set_style_pad_right(root, 10, 0);
    lv_obj_set_style_pad_top(root, 10, 0);
    lv_obj_set_style_pad_bottom(root, 10, 0);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scroll_dir(root, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_ON);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_PRESS_LOCK);

    create_label(root, "Board", true);
    create_label(root, "ESP-Brookesia launcher profile for Waveshare ESP32-S3 Touch LCD 2.8 V1.");
    create_info_row(root, "Target", CONFIG_IDF_TARGET);
    create_info_row(root, "Display", "240 x 320 RGB565, ST7789");
    create_info_row(root, "Touch", "CST328 pointer input");
    create_info_row(root, "Storage", "SD/MMC, FATFS, SPIFFS, LittleFS");
    create_info_row(root, "Sensors", "PCF85063 RTC, QMI8658 IMU");
    create_info_row(root, "Audio", "PCM5101 I2S, default volume 50%");
    create_info_row(root, "Navigation", "Fixed Back / Home / Recents bar");
    create_info_row(root, "CPU", "ESP32-S3 at 240 MHz");
    create_info_row(root, "Flash", "16 MB external flash");
    create_info_row(root, "PSRAM", "Octal PSRAM enabled");
    create_info_row(root, "Display driver", "ST7789 over SPI");
    create_info_row(root, "Backlight", "LEDC PWM");
    create_info_row(root, "Board revision", "Waveshare V1 profile");

    return true;
}

bool PhoneAppBoardInfo::back(void)
{
    return notifyCoreClosed();
}
