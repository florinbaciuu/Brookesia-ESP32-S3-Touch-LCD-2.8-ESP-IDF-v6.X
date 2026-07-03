#include "phone_app_storage.hpp"

#include <stdio.h>
#include <string.h>

extern "C" {
#include "defines.h"
#include "esp_err.h"
#include "esp_littlefs.h"
#include "esp_spiffs.h"
#include "esp_vfs_fat.h"
#include "lvgl.h"
}

namespace {

typedef struct {
    const char* name;
    const char* mount_path;
    const char* detail;
    lv_obj_t* status_label;
    lv_obj_t* usage_label;
    lv_obj_t* bar;
} storage_row_t;

static storage_row_t s_rows[] = {
    { "SD card", SD_MOUNT_PATH, "External FAT storage", nullptr, nullptr, nullptr },
    { "Internal FAT", FAT_MOUNT_PATH, "Wear-levelled flash FAT", nullptr, nullptr, nullptr },
    { "SPIFFS", SPIFFS_MOUNT_PATH, "Read/write flash partition", nullptr, nullptr, nullptr },
    { "LittleFS", LITTLEFS_MOUNT_PATH, "Read/write flash partition", nullptr, nullptr, nullptr },
};

ESP_Brookesia_CoreAppData_t create_storage_core_data()
{
    return ESP_BROOKESIA_CORE_APP_DATA_DEFAULT("Storage", nullptr, true);
}

ESP_Brookesia_PhoneAppData_t create_storage_phone_data()
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

void format_bytes(uint64_t bytes, char* buffer, size_t buffer_size)
{
    if (buffer == nullptr || buffer_size == 0) {
        return;
    }

    if (bytes >= 1024ULL * 1024ULL * 1024ULL) {
        snprintf(buffer, buffer_size, "%.2f GB", (double)bytes / (1024.0 * 1024.0 * 1024.0));
    } else if (bytes >= 1024ULL * 1024ULL) {
        snprintf(buffer, buffer_size, "%.1f MB", (double)bytes / (1024.0 * 1024.0));
    } else if (bytes >= 1024ULL) {
        snprintf(buffer, buffer_size, "%.1f KB", (double)bytes / 1024.0);
    } else {
        snprintf(buffer, buffer_size, "%llu B", (unsigned long long)bytes);
    }
}

esp_err_t get_storage_capacity(const char* mount_path, uint64_t* total_bytes, uint64_t* used_bytes, uint64_t* free_bytes)
{
    if (mount_path == nullptr || total_bytes == nullptr || used_bytes == nullptr || free_bytes == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    *total_bytes = 0;
    *used_bytes = 0;
    *free_bytes = 0;

    if (strcmp(mount_path, SD_MOUNT_PATH) == 0 || strcmp(mount_path, FAT_MOUNT_PATH) == 0) {
        uint64_t total = 0;
        uint64_t free = 0;
        const esp_err_t err = esp_vfs_fat_info(mount_path, &total, &free);
        if (err != ESP_OK) {
            return err;
        }
        *total_bytes = total;
        *free_bytes = free;
        *used_bytes = total >= free ? total - free : 0;
        return ESP_OK;
    }

    if (strcmp(mount_path, SPIFFS_MOUNT_PATH) == 0) {
        size_t total = 0;
        size_t used = 0;
        const esp_err_t err = esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used);
        if (err != ESP_OK) {
            return err;
        }
        *total_bytes = total;
        *used_bytes = used;
        *free_bytes = total >= used ? total - used : 0;
        return ESP_OK;
    }

    if (strcmp(mount_path, LITTLEFS_MOUNT_PATH) == 0) {
        size_t total = 0;
        size_t used = 0;
        const esp_err_t err = esp_littlefs_info(LITTLEFS_PARTITION_LABEL, &total, &used);
        if (err != ESP_OK) {
            return err;
        }
        *total_bytes = total;
        *used_bytes = used;
        *free_bytes = total >= used ? total - used : 0;
        return ESP_OK;
    }

    return ESP_ERR_NOT_SUPPORTED;
}

void update_row(storage_row_t* row)
{
    if (row == nullptr || row->status_label == nullptr || row->usage_label == nullptr || row->bar == nullptr) {
        return;
    }

    uint64_t total = 0;
    uint64_t used = 0;
    uint64_t free = 0;
    const esp_err_t err = get_storage_capacity(row->mount_path, &total, &used, &free);
    if (err != ESP_OK || total == 0) {
        lv_label_set_text_fmt(row->status_label, "%s\n%s", row->mount_path, row->detail);
        lv_label_set_text_fmt(row->usage_label, "Not mounted: %s", esp_err_to_name(err));
        lv_bar_set_value(row->bar, 0, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(row->bar, lv_color_hex(0x2D3B47), LV_PART_INDICATOR);
        return;
    }

    char total_text[24] = { 0 };
    char used_text[24] = { 0 };
    char free_text[24] = { 0 };
    format_bytes(total, total_text, sizeof(total_text));
    format_bytes(used, used_text, sizeof(used_text));
    format_bytes(free, free_text, sizeof(free_text));

    const int percent = (int)((used * 100ULL) / total);
    lv_label_set_text_fmt(row->status_label, "%s\n%s", row->mount_path, row->detail);
    lv_label_set_text_fmt(row->usage_label, "%s used / %s total\n%s free", used_text, total_text, free_text);
    lv_bar_set_value(row->bar, percent, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(row->bar, lv_color_hex(percent > 85 ? 0xD45757 : 0x41B883), LV_PART_INDICATOR);
}

void refresh_storage_rows()
{
    for (storage_row_t& row : s_rows) {
        update_row(&row);
    }
}

void refresh_event_cb(lv_event_t* event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
        refresh_storage_rows();
    }
}

lv_obj_t* create_storage_row(lv_obj_t* parent, storage_row_t* row)
{
    lv_obj_t* container = lv_obj_create(parent);
    if (container == nullptr || row == nullptr) {
        return nullptr;
    }

    lv_obj_set_width(container, lv_pct(100));
    lv_obj_set_height(container, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x17232E), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_radius(container, 6, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_gap(container, 5, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_EVENT_BUBBLE | LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_t* title = lv_label_create(container);
    if (title != nullptr) {
        lv_label_set_text(title, row->name);
        lv_obj_set_width(title, lv_pct(100));
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    }

    row->status_label = lv_label_create(container);
    if (row->status_label != nullptr) {
        lv_label_set_long_mode(row->status_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(row->status_label, lv_pct(100));
        lv_obj_set_style_text_color(row->status_label, lv_color_hex(0x8DA2B8), 0);
    }

    row->bar = lv_bar_create(container);
    if (row->bar != nullptr) {
        lv_obj_set_width(row->bar, lv_pct(100));
        lv_obj_set_height(row->bar, 8);
        lv_bar_set_range(row->bar, 0, 100);
        lv_obj_set_style_bg_color(row->bar, lv_color_hex(0x2D3B47), 0);
    }

    row->usage_label = lv_label_create(container);
    if (row->usage_label != nullptr) {
        lv_label_set_long_mode(row->usage_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(row->usage_label, lv_pct(100));
        lv_obj_set_style_text_color(row->usage_label, lv_color_hex(0xD6DEE8), 0);
    }

    return container;
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

PhoneAppStorage::PhoneAppStorage()
    : ESP_Brookesia_PhoneApp(create_storage_core_data(), create_storage_phone_data())
{
}

bool PhoneAppStorage::run(void)
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
    lv_obj_set_style_pad_gap(root, 8, 0);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_scroll_dir(root, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_AUTO);

    create_label(root, "Storage", true);
    create_label(root, "Mounted filesystems");
    create_refresh_button(root);

    for (storage_row_t& row : s_rows) {
        create_storage_row(root, &row);
    }

    refresh_storage_rows();
    return true;
}

bool PhoneAppStorage::back(void)
{
    return notifyCoreClosed();
}
