#include "phone_app_sensors.hpp"

#include <stdio.h>

extern "C" {
#include "esp_err.h"
#include "lvgl.h"
#include "power_key.h"
#include "qmi8658.h"
#include "rtc_pcf85063.h"
}

namespace {

typedef struct {
    lv_obj_t* rtc;
    lv_obj_t* accel;
    lv_obj_t* gyro;
    lv_obj_t* power;
    lv_obj_t* status;
} sensors_labels_t;

static sensors_labels_t s_labels = {};

ESP_Brookesia_CoreAppData_t create_sensors_core_data()
{
    return ESP_BROOKESIA_CORE_APP_DATA_DEFAULT("Sensors", nullptr, true);
}

ESP_Brookesia_PhoneAppData_t create_sensors_phone_data()
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
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_bottom(row, 8, 0);
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
        lv_label_set_text(value_label, "--");
        lv_label_set_long_mode(value_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(value_label, lv_pct(100));
        lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(value_label, LV_FONT_DEFAULT, 0);
    }

    return value_label;
}

void set_label(lv_obj_t* label, const char* text)
{
    if (label != nullptr) {
        lv_label_set_text(label, text);
    }
}

void update_sensor_labels(sensors_labels_t* labels)
{
    if (labels == nullptr) {
        return;
    }

    char buffer[96] = { 0 };
    rtc_pcf85063_datetime_t time = {};
    const esp_err_t rtc_err = rtc_pcf85063_read(&time);
    if (rtc_err == ESP_OK) {
        rtc_pcf85063_format(&time, buffer, sizeof(buffer));
    } else {
        snprintf(buffer, sizeof(buffer), "RTC error: %s", esp_err_to_name(rtc_err));
    }
    set_label(labels->rtc, buffer);

    qmi8658_sample_t sample = {};
    const esp_err_t imu_err = qmi8658_read(&sample);
    if (imu_err == ESP_OK) {
        snprintf(buffer,
            sizeof(buffer),
            "X %.2fg  Y %.2fg  Z %.2fg",
            (double)sample.accel_g.x,
            (double)sample.accel_g.y,
            (double)sample.accel_g.z);
        set_label(labels->accel, buffer);

        snprintf(buffer,
            sizeof(buffer),
            "X %.1f  Y %.1f  Z %.1f dps",
            (double)sample.gyro_dps.x,
            (double)sample.gyro_dps.y,
            (double)sample.gyro_dps.z);
        set_label(labels->gyro, buffer);

        snprintf(buffer, sizeof(buffer), "QMI8658 0x%02x who=0x%02x rev=0x%02x", sample.address, sample.who_am_i, sample.revision);
        set_label(labels->status, buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "IMU error: %s", esp_err_to_name(imu_err));
        set_label(labels->accel, buffer);
        set_label(labels->gyro, "--");
        set_label(labels->status, "QMI8658 unavailable");
    }

    power_key_status_t power_status = {};
    const esp_err_t power_err = power_key_get_status(&power_status);
    if (power_err == ESP_OK) {
        snprintf(buffer,
            sizeof(buffer),
            "Key %s, latch %s",
            power_status.key_pressed ? "pressed" : "released",
            power_status.latch_enabled ? "on" : "off");
    } else {
        snprintf(buffer, sizeof(buffer), "Power key error: %s", esp_err_to_name(power_err));
    }
    set_label(labels->power, buffer);
}

void update_timer_cb(lv_timer_t* timer)
{
    sensors_labels_t* labels = static_cast<sensors_labels_t*>(lv_timer_get_user_data(timer));
    update_sensor_labels(labels);
}

} // namespace

PhoneAppSensors::PhoneAppSensors()
    : ESP_Brookesia_PhoneApp(create_sensors_core_data(), create_sensors_phone_data())
{
}

bool PhoneAppSensors::run(void)
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
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_scroll_dir(root, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_AUTO);

    create_label(root, "Sensors", true);
    create_label(root, "Live board peripherals");

    s_labels.rtc = create_value_row(root, "RTC");
    s_labels.accel = create_value_row(root, "Accelerometer");
    s_labels.gyro = create_value_row(root, "Gyroscope");
    s_labels.power = create_value_row(root, "Power key");
    s_labels.status = create_value_row(root, "Status");

    update_sensor_labels(&s_labels);
    lv_timer_create(update_timer_cb, 500, &s_labels);

    return true;
}

bool PhoneAppSensors::back(void)
{
    return notifyCoreClosed();
}
