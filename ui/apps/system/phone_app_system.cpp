#include "phone_app_system.hpp"

#include <stdio.h>

extern "C" {
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "lvgl.h"
}

namespace {

typedef struct {
    lv_obj_t* uptime;
    lv_obj_t* chip;
    lv_obj_t* flash;
    lv_obj_t* internal_heap;
    lv_obj_t* psram;
} system_labels_t;

static system_labels_t s_labels = {};

ESP_Brookesia_CoreAppData_t create_system_core_data()
{
    return ESP_BROOKESIA_CORE_APP_DATA_DEFAULT("System", nullptr, true);
}

ESP_Brookesia_PhoneAppData_t create_system_phone_data()
{
    ESP_Brookesia_PhoneAppData_t data = ESP_BROOKESIA_PHONE_APP_DATA_DEFAULT(nullptr, true, false);
    data.flags.enable_navigation_gesture = 1;
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

lv_obj_t* create_value_row(lv_obj_t* parent, const char* title)
{
    lv_obj_t* row = lv_obj_create(parent);
    if (row == nullptr) {
        return nullptr;
    }

    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x17232E), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_radius(row, 6, 0);
    lv_obj_set_style_pad_all(row, 8, 0);
    lv_obj_set_style_pad_gap(row, 5, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(row, LV_OBJ_FLAG_EVENT_BUBBLE | LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_t* title_label = lv_label_create(row);
    if (title_label != nullptr) {
        lv_label_set_text(title_label, title);
        lv_obj_set_width(title_label, lv_pct(100));
        lv_obj_set_style_text_color(title_label, lv_color_hex(0x8DA2B8), 0);
    }

    lv_obj_t* value_label = lv_label_create(row);
    if (value_label != nullptr) {
        lv_label_set_text(value_label, "--");
        lv_label_set_long_mode(value_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(value_label, lv_pct(100));
        lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), 0);
    }

    return value_label;
}

void format_bytes(size_t bytes, char* buffer, size_t buffer_size)
{
    if (buffer == nullptr || buffer_size == 0) {
        return;
    }

    if (bytes >= 1024U * 1024U) {
        snprintf(buffer, buffer_size, "%.1f MB", (double)bytes / (1024.0 * 1024.0));
    } else if (bytes >= 1024U) {
        snprintf(buffer, buffer_size, "%.1f KB", (double)bytes / 1024.0);
    } else {
        snprintf(buffer, buffer_size, "%u B", (unsigned)bytes);
    }
}

void set_heap_label(lv_obj_t* label, uint32_t caps)
{
    if (label == nullptr) {
        return;
    }

    const size_t total = heap_caps_get_total_size(caps);
    const size_t free = heap_caps_get_free_size(caps);
    const size_t largest = heap_caps_get_largest_free_block(caps);
    const size_t used = total >= free ? total - free : 0;

    char total_text[24] = { 0 };
    char used_text[24] = { 0 };
    char free_text[24] = { 0 };
    char largest_text[24] = { 0 };
    format_bytes(total, total_text, sizeof(total_text));
    format_bytes(used, used_text, sizeof(used_text));
    format_bytes(free, free_text, sizeof(free_text));
    format_bytes(largest, largest_text, sizeof(largest_text));

    lv_label_set_text_fmt(label,
        "%s used / %s total\n%s free, largest %s",
        used_text,
        total_text,
        free_text,
        largest_text);
}

void update_system_labels()
{
    const int64_t uptime_sec = esp_timer_get_time() / 1000000LL;
    const int64_t hours = uptime_sec / 3600;
    const int64_t minutes = (uptime_sec % 3600) / 60;
    const int64_t seconds = uptime_sec % 60;
    if (s_labels.uptime != nullptr) {
        lv_label_set_text_fmt(s_labels.uptime, "%02lld:%02lld:%02lld", hours, minutes, seconds);
    }

    esp_chip_info_t chip_info = {};
    esp_chip_info(&chip_info);
    if (s_labels.chip != nullptr) {
        lv_label_set_text_fmt(s_labels.chip,
            "%d core(s), rev %d\nFeatures: WiFi%s%s",
            chip_info.cores,
            chip_info.revision,
            (chip_info.features & CHIP_FEATURE_BT) ? ", BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? ", BLE" : "");
    }

    uint32_t flash_size = 0;
    if (s_labels.flash != nullptr) {
        if (esp_flash_get_size(nullptr, &flash_size) == ESP_OK) {
            char flash_text[24] = { 0 };
            format_bytes(flash_size, flash_text, sizeof(flash_text));
            lv_label_set_text_fmt(s_labels.flash, "%s external flash", flash_text);
        } else {
            lv_label_set_text(s_labels.flash, "Flash size unavailable");
        }
    }

    set_heap_label(s_labels.internal_heap, MALLOC_CAP_INTERNAL);
    set_heap_label(s_labels.psram, MALLOC_CAP_SPIRAM);
}

void refresh_event_cb(lv_event_t* event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
        update_system_labels();
    }
}

lv_obj_t* create_refresh_button(lv_obj_t* parent)
{
    lv_obj_t* button = lv_button_create(parent);
    if (button == nullptr) {
        return nullptr;
    }

    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 34);
    lv_obj_add_event_cb(button, refresh_event_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* label = lv_label_create(button);
    if (label != nullptr) {
        lv_label_set_text(label, "Refresh");
        lv_obj_center(label);
    }

    return button;
}

} // namespace

PhoneAppSystem::PhoneAppSystem()
    : ESP_Brookesia_PhoneApp(create_system_core_data(), create_system_phone_data())
{
}

bool PhoneAppSystem::run(void)
{
    lv_obj_t* screen = lv_scr_act();
    if (screen == nullptr) {
        return false;
    }

    s_labels = {};

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
    lv_obj_set_style_pad_gap(root, 8, 0);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_scroll_dir(root, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_AUTO);

    create_label(root, "System", true);
    create_label(root, "Runtime status");
    create_refresh_button(root);

    s_labels.uptime = create_value_row(root, "Uptime");
    s_labels.chip = create_value_row(root, "Chip");
    s_labels.flash = create_value_row(root, "Flash");
    s_labels.internal_heap = create_value_row(root, "Internal heap");
    s_labels.psram = create_value_row(root, "PSRAM");

    update_system_labels();
    return true;
}

bool PhoneAppSystem::back(void)
{
    return notifyCoreClosed();
}
