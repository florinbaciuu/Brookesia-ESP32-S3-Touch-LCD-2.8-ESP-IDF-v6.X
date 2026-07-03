#include "phone_app_settings.hpp"

#include <stdio.h>

#include "systems/phone/esp_brookesia_phone.hpp"

extern "C" {
#include "audio_pcm5101.h"
#include "esp_err.h"
#include "lcd_backlight.h"
#include "lvgl.h"
#include "power_key.h"
#include "settings_nvs.h"
}

namespace {

typedef struct {
    lv_obj_t* brightness_value;
    lv_obj_t* volume_value;
    lv_obj_t* power_value;
    lv_obj_t* nav_value;
    PhoneAppSettings* app;
} settings_context_t;

static settings_context_t s_settings = {};
static bool s_navigation_bar_visible = false;
static settings_nvs_values_t s_saved_values = {};

ESP_Brookesia_CoreAppData_t create_settings_core_data()
{
    return ESP_BROOKESIA_CORE_APP_DATA_DEFAULT("Settings", nullptr, true);
}

ESP_Brookesia_PhoneAppData_t create_settings_phone_data()
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

lv_obj_t* create_setting_row(lv_obj_t* parent, const char* title)
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

    lv_obj_t* label = lv_label_create(row);
    if (label != nullptr) {
        lv_label_set_text(label, title);
        lv_obj_set_width(label, lv_pct(100));
        lv_obj_set_style_text_color(label, lv_color_hex(0x8DA2B8), 0);
    }

    return row;
}

lv_obj_t* create_value_label(lv_obj_t* parent)
{
    lv_obj_t* label = lv_label_create(parent);
    if (label == nullptr) {
        return nullptr;
    }

    lv_label_set_text(label, "--");
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    return label;
}

lv_obj_t* create_slider(lv_obj_t* parent, int32_t min, int32_t max, int32_t value)
{
    lv_obj_t* slider = lv_slider_create(parent);
    if (slider == nullptr) {
        return nullptr;
    }

    lv_obj_set_width(slider, lv_pct(100));
    lv_obj_set_height(slider, 22);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, value, LV_ANIM_OFF);
    return slider;
}

void set_value_fmt(lv_obj_t* label, const char* fmt, int value)
{
    if (label != nullptr) {
        lv_label_set_text_fmt(label, fmt, value);
    }
}

bool is_slider_save_event(lv_event_code_t code)
{
    return code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST || code == LV_EVENT_CANCEL;
}

void apply_navigation_bar_visible(bool visible)
{
    s_navigation_bar_visible = visible;

    ESP_Brookesia_Phone* phone = s_settings.app == nullptr ? nullptr : s_settings.app->getPhone();
    if (phone != nullptr && phone->getHome().getNavigationBar() != nullptr) {
        phone->getHome().getNavigationBar()->setVisualMode(
            visible ? ESP_BROOKESIA_NAVIGATION_BAR_VISUAL_MODE_SHOW_FIXED :
                      ESP_BROOKESIA_NAVIGATION_BAR_VISUAL_MODE_HIDE);
    }

    if (s_settings.nav_value != nullptr) {
        lv_label_set_text(s_settings.nav_value, visible ? "Fixed bar visible" : "Gestures only");
    }
}

void brightness_event_cb(lv_event_t* event)
{
    const lv_event_code_t code = lv_event_get_code(event);
    if (code != LV_EVENT_VALUE_CHANGED && !is_slider_save_event(code)) {
        return;
    }

    lv_obj_t* slider = lv_event_get_target_obj(event);
    const int32_t value = lv_slider_get_value(slider);
    Backlight_Set((uint8_t)value);
    set_value_fmt(s_settings.brightness_value, "%d%%", (int)value);

    if (is_slider_save_event(code)) {
        (void)settings_nvs_set_brightness((uint8_t)value);
    }
}

void volume_event_cb(lv_event_t* event)
{
    const lv_event_code_t code = lv_event_get_code(event);
    if (code != LV_EVENT_VALUE_CHANGED && !is_slider_save_event(code)) {
        return;
    }

    lv_obj_t* slider = lv_event_get_target_obj(event);
    const int32_t value = lv_slider_get_value(slider);
    (void)audio_pcm5101_set_volume((uint8_t)value);
    set_value_fmt(s_settings.volume_value, "%d%%", (int)value);

    if (is_slider_save_event(code)) {
        (void)settings_nvs_set_volume((uint8_t)value);
    }
}

void test_tone_event_cb(lv_event_t* event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
        (void)audio_pcm5101_play_test_tone(180, 660);
    }
}

void power_latch_event_cb(lv_event_t* event)
{
    if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED) {
        return;
    }

    lv_obj_t* sw = lv_event_get_target_obj(event);
    const bool enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
    const esp_err_t err = power_key_set_latch(enabled);
    if (s_settings.power_value != nullptr) {
        if (err == ESP_OK) {
            (void)settings_nvs_set_power_latch(enabled);
            lv_label_set_text(s_settings.power_value, enabled ? "Latch on" : "Latch off");
        } else {
            lv_label_set_text_fmt(s_settings.power_value, "Error: %s", esp_err_to_name(err));
        }
    }
}

void navigation_bar_event_cb(lv_event_t* event)
{
    if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED || s_settings.app == nullptr) {
        return;
    }

    lv_obj_t* sw = lv_event_get_target_obj(event);
    apply_navigation_bar_visible(lv_obj_has_state(sw, LV_STATE_CHECKED));
    (void)settings_nvs_set_navigation_bar(s_navigation_bar_visible);
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

PhoneAppSettings::PhoneAppSettings()
    : ESP_Brookesia_PhoneApp(create_settings_core_data(), create_settings_phone_data())
{
}

bool PhoneAppSettings::run(void)
{
    lv_obj_t* screen = lv_scr_act();
    if (screen == nullptr) {
        return false;
    }

    s_settings = {};
    s_settings.app = this;

    s_saved_values = {};
    if (settings_nvs_load(&s_saved_values) != ESP_OK) {
        s_saved_values.brightness_percent = s_backlight_level;

        audio_pcm5101_status_t audio_status = {};
        (void)audio_pcm5101_get_status(&audio_status);
        s_saved_values.volume_percent = audio_status.volume_percent;

        power_key_status_t power_status = {};
        (void)power_key_get_status(&power_status);
        s_saved_values.power_latch_enabled = power_status.latch_enabled;
        s_saved_values.navigation_bar_visible = s_navigation_bar_visible;
    }

    Backlight_Set(s_saved_values.brightness_percent);
    (void)audio_pcm5101_set_volume(s_saved_values.volume_percent);
    (void)power_key_set_latch(s_saved_values.power_latch_enabled);
    s_navigation_bar_visible = s_saved_values.navigation_bar_visible;

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

    create_label(root, "Settings", true);
    create_label(root, "Board controls");

    lv_obj_t* brightness_row = create_setting_row(root, "Backlight");
    if (brightness_row != nullptr) {
        s_settings.brightness_value = create_value_label(brightness_row);
        set_value_fmt(s_settings.brightness_value, "%d%%", s_saved_values.brightness_percent);
        lv_obj_t* slider = create_slider(brightness_row, 5, 100, s_saved_values.brightness_percent);
        if (slider != nullptr) {
            lv_obj_add_event_cb(slider, brightness_event_cb, LV_EVENT_ALL, nullptr);
        }
    }

    lv_obj_t* volume_row = create_setting_row(root, "Audio volume");
    if (volume_row != nullptr) {
        s_settings.volume_value = create_value_label(volume_row);
        set_value_fmt(s_settings.volume_value, "%d%%", s_saved_values.volume_percent);
        lv_obj_t* slider = create_slider(volume_row, 0, 100, s_saved_values.volume_percent);
        if (slider != nullptr) {
            lv_obj_add_event_cb(slider, volume_event_cb, LV_EVENT_ALL, nullptr);
        }
        create_button(volume_row, "Test tone", test_tone_event_cb);
    }

    lv_obj_t* power_row = create_setting_row(root, "Power latch");
    if (power_row != nullptr) {
        s_settings.power_value = create_value_label(power_row);
        lv_label_set_text(s_settings.power_value, s_saved_values.power_latch_enabled ? "Latch on" : "Latch off");
        lv_obj_t* sw = lv_switch_create(power_row);
        if (sw != nullptr) {
            if (s_saved_values.power_latch_enabled) {
                lv_obj_add_state(sw, LV_STATE_CHECKED);
            }
            lv_obj_add_event_cb(sw, power_latch_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
        }
    }

    lv_obj_t* nav_row = create_setting_row(root, "Navigation bar");
    if (nav_row != nullptr) {
        s_settings.nav_value = create_value_label(nav_row);
        apply_navigation_bar_visible(s_navigation_bar_visible);
        lv_obj_t* sw = lv_switch_create(nav_row);
        if (sw != nullptr) {
            if (s_navigation_bar_visible) {
                lv_obj_add_state(sw, LV_STATE_CHECKED);
            }
            lv_obj_add_event_cb(sw, navigation_bar_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
        }
    }

    return true;
}

bool PhoneAppSettings::back(void)
{
    return notifyCoreClosed();
}
