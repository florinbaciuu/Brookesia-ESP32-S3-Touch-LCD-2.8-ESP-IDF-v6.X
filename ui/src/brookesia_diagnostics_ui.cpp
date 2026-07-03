#include "brookesia_diagnostics_ui.h"

#include <inttypes.h>
#include <stdio.h>

extern "C" {
#include "esp_timer.h"
#include "lvgl.h"
}

static lv_obj_t* s_status_label = nullptr;
static lv_obj_t* s_uptime_label = nullptr;
static brookesia_diagnostics_status_provider_t s_status_provider = nullptr;

static void status_timer_cb(lv_timer_t* timer) {
    (void) timer;

    const int64_t uptime_s = esp_timer_get_time() / 1000000;
    if (s_status_label != nullptr && s_status_provider != nullptr) {
        char status[448];
        s_status_provider(status, sizeof(status));
        lv_label_set_text(s_status_label, status);
    }
    if (s_uptime_label != nullptr) {
        lv_label_set_text_fmt(s_uptime_label, "Uptime %" PRId64 " s", uptime_s);
    }
}

void brookesia_diagnostics_ui_create(brookesia_diagnostics_status_provider_t status_provider) {
    s_status_provider = status_provider;

    lv_obj_t* screen = lv_screen_active();
    lv_obj_clean(screen);

    lv_obj_t* root = lv_obj_create(screen);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(root, lv_color_hex(0x111827), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(root, 12, 0);
    lv_obj_set_style_pad_row(root, 8, 0);
    lv_obj_set_scroll_dir(root, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_AUTO);

    lv_obj_t* title = lv_label_create(root);
    lv_label_set_text(title, "Board Diagnostics");
    lv_obj_set_width(title, LV_PCT(100));
    lv_obj_set_style_text_color(title, lv_color_hex(0xF9FAFB), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, 0);

    s_uptime_label = lv_label_create(root);
    lv_label_set_text(s_uptime_label, "Uptime 0 s");
    lv_obj_set_width(s_uptime_label, LV_PCT(100));
    lv_obj_set_style_text_color(s_uptime_label, lv_color_hex(0x93C5FD), 0);
    lv_obj_set_style_text_align(s_uptime_label, LV_TEXT_ALIGN_LEFT, 0);

    lv_obj_t* divider = lv_obj_create(root);
    lv_obj_remove_style_all(divider);
    lv_obj_set_size(divider, LV_PCT(100), 1);
    lv_obj_set_style_bg_color(divider, lv_color_hex(0x334155), 0);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, 0);

    s_status_label = lv_label_create(root);
    char status[448];
    if (s_status_provider != nullptr) {
        s_status_provider(status, sizeof(status));
    } else {
        snprintf(status, sizeof(status), "Board status provider not available");
    }
    lv_label_set_text(s_status_label, status);
    lv_obj_set_width(s_status_label, LV_PCT(100));
    lv_label_set_long_mode(s_status_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(s_status_label, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_text_align(s_status_label, LV_TEXT_ALIGN_LEFT, 0);

    lv_timer_create(status_timer_cb, 1000, nullptr);
}
