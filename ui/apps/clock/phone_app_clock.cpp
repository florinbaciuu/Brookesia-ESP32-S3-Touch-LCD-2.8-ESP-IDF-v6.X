#include "phone_app_clock.hpp"

extern "C" {
#include "esp_err.h"
#include "lvgl.h"
#include "rtc_pcf85063.h"
}

namespace {

typedef struct {
    lv_obj_t* time;
    lv_obj_t* date;
    lv_obj_t* status;
} clock_labels_t;

static clock_labels_t s_labels = {};

ESP_Brookesia_CoreAppData_t create_clock_core_data()
{
    return ESP_BROOKESIA_CORE_APP_DATA_DEFAULT("Clock", nullptr, true);
}

ESP_Brookesia_PhoneAppData_t create_clock_phone_data()
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

lv_obj_t* create_panel(lv_obj_t* parent)
{
    lv_obj_t* panel = lv_obj_create(parent);
    if (panel == nullptr) {
        return nullptr;
    }

    lv_obj_set_width(panel, lv_pct(100));
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x17232E), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_radius(panel, 6, 0);
    lv_obj_set_style_pad_all(panel, 10, 0);
    lv_obj_set_style_pad_gap(panel, 8, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE | LV_OBJ_FLAG_GESTURE_BUBBLE);
    return panel;
}

void update_clock_labels()
{
    rtc_pcf85063_datetime_t rtc_time = {};
    const esp_err_t err = rtc_pcf85063_read(&rtc_time);
    if (err != ESP_OK) {
        if (s_labels.time != nullptr) {
            lv_label_set_text(s_labels.time, "--:--:--");
        }
        if (s_labels.date != nullptr) {
            lv_label_set_text(s_labels.date, "RTC unavailable");
        }
        if (s_labels.status != nullptr) {
            lv_label_set_text_fmt(s_labels.status, "PCF85063 error: %s", esp_err_to_name(err));
        }
        return;
    }

    if (!rtc_pcf85063_datetime_is_valid(&rtc_time)) {
        if (s_labels.time != nullptr) {
            lv_label_set_text(s_labels.time, "--:--:--");
        }
        if (s_labels.date != nullptr) {
            lv_label_set_text(s_labels.date, "Invalid RTC date");
        }
        if (s_labels.status != nullptr) {
            lv_label_set_text(s_labels.status, "Use build-time sync after flashing if the backup battery was removed.");
        }
        return;
    }

    if (s_labels.time != nullptr) {
        lv_label_set_text_fmt(s_labels.time,
            "%02u:%02u:%02u",
            (unsigned)rtc_time.hour,
            (unsigned)rtc_time.minute,
            (unsigned)rtc_time.second);
    }
    if (s_labels.date != nullptr) {
        lv_label_set_text_fmt(s_labels.date,
            "%04u-%02u-%02u",
            (unsigned)rtc_time.year,
            (unsigned)rtc_time.month,
            (unsigned)rtc_time.day);
    }
    if (s_labels.status != nullptr) {
        lv_label_set_text_fmt(s_labels.status, "Weekday %u, PCF85063 active", (unsigned)rtc_time.weekday);
    }
}

void refresh_event_cb(lv_event_t* event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
        update_clock_labels();
    }
}

void sync_build_time_event_cb(lv_event_t* event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    const esp_err_t err = rtc_pcf85063_set_build_time();
    if (s_labels.status != nullptr) {
        if (err == ESP_OK) {
            lv_label_set_text(s_labels.status, "RTC set from firmware build time");
        } else {
            lv_label_set_text_fmt(s_labels.status, "Build-time sync failed: %s", esp_err_to_name(err));
        }
    }
    update_clock_labels();
}

lv_obj_t* create_button(lv_obj_t* parent, const char* text, lv_event_cb_t cb)
{
    lv_obj_t* button = lv_button_create(parent);
    if (button == nullptr) {
        return nullptr;
    }

    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 34);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* label = lv_label_create(button);
    if (label != nullptr) {
        lv_label_set_text(label, text);
        lv_obj_center(label);
    }

    return button;
}

} // namespace

PhoneAppClock::PhoneAppClock()
    : ESP_Brookesia_PhoneApp(create_clock_core_data(), create_clock_phone_data())
{
}

bool PhoneAppClock::run(void)
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

    create_label(root, "Clock", true);
    create_label(root, "PCF85063 RTC");

    lv_obj_t* panel = create_panel(root);
    if (panel != nullptr) {
        s_labels.time = create_label(panel, "--:--:--", true);
        if (s_labels.time != nullptr) {
            lv_obj_set_style_text_align(s_labels.time, LV_TEXT_ALIGN_CENTER, 0);
        }
        s_labels.date = create_label(panel, "---- -- --");
        if (s_labels.date != nullptr) {
            lv_obj_set_style_text_align(s_labels.date, LV_TEXT_ALIGN_CENTER, 0);
        }
        s_labels.status = create_label(panel, "--");
    }

    create_button(root, "Refresh", refresh_event_cb);
    create_button(root, "Set RTC from build time", sync_build_time_event_cb);

    update_clock_labels();
    return true;
}

bool PhoneAppClock::back(void)
{
    return notifyCoreClosed();
}
