// made with LVGL.
// by Florin Baciu, 2025

#include "app_tabs_ui.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "lvgl_framework.h"
}

static const char* TAG = "UI";

static lv_obj_t* label_drift       = NULL;
static lv_obj_t* slider_tab4_label = NULL;

static void lv_drift_timer_cb(lv_timer_t* timer) {
    (void) timer;

    static int64_t  real0_us = 0;
    static uint32_t lv0_ms   = 0;
    static char     last_buf[96] = "";

    if (real0_us == 0) {
        real0_us = esp_timer_get_time();
        lv0_ms   = lv_tick_get();
        return;
    }

    const int64_t  real_ms = (esp_timer_get_time() - real0_us) / 1000;
    const uint32_t lv_ms   = lv_tick_get() - lv0_ms;

    char buf[96];
    snprintf(buf,
        sizeof(buf),
        "Real: %" PRId64 " ms\nLVGL: %" PRIu32 " ms\nDrift: %" PRId64 " ms",
        real_ms,
        lv_ms,
        (int64_t) lv_ms - real_ms);

    if (strcmp(buf, last_buf) != 0 && label_drift != NULL) {
        strlcpy(last_buf, buf, sizeof(last_buf));
        lv_label_set_text(label_drift, buf);
    }
}

static void btn1_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Hello World");
    }
}

static void btn3_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Hello People");
    }
}

static void btn_rotate_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    lv_display_t* disp = lv_display_get_default();
    int rotation       = (int) lv_display_get_rotation(disp) + 1;
    if (rotation > (int) LV_DISPLAY_ROTATION_270) {
        rotation = (int) LV_DISPLAY_ROTATION_0;
    }

    lv_display_set_rotation(disp, (lv_display_rotation_t) rotation);
    lvgl_displ_rotate_now();
    ESP_LOGI(TAG, "Display rotation changed to %d", rotation);
}

static void slider_event_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target_obj(e);
    lv_label_set_text_fmt(slider_tab4_label, "%" LV_PRId32, lv_slider_get_value(slider));
    lv_obj_align_to(slider_tab4_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);
}

void create_tabs_ui(void) {
    lv_obj_t* tabview = lv_tabview_create(lv_screen_active());
    lv_tabview_set_tab_bar_size(tabview, 40);
    lv_obj_set_size(tabview, LV_PCT(100), LV_PCT(100));
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_TOP);

    lv_obj_t* tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t* tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t* tab3 = lv_tabview_add_tab(tabview, "Tab 3");
    lv_obj_t* tab4 = lv_tabview_add_tab(tabview, "Tab 4");

    lv_obj_t* container = lv_obj_create(tab1);
    lv_obj_center(container);
    lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(container, 15, 0);

    lv_obj_t* btn1 = lv_button_create(container);
    lv_obj_add_event_cb(btn1, btn1_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* btn1_label = lv_label_create(btn1);
    lv_label_set_text(btn1_label, "Hello World");
    lv_obj_center(btn1_label);

    lv_obj_t* btn_rotate = lv_button_create(container);
    lv_obj_add_event_cb(btn_rotate, btn_rotate_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* btn_rotate_label = lv_label_create(btn_rotate);
    lv_label_set_text(btn_rotate_label, "Rotate Display");
    lv_obj_center(btn_rotate_label);

    lv_obj_t* btn3 = lv_button_create(tab2);
    lv_obj_center(btn3);
    lv_obj_add_event_cb(btn3, btn3_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* btn3_label = lv_label_create(btn3);
    lv_label_set_text(btn3_label, "Hello People");
    lv_obj_center(btn3_label);

    lv_obj_t* tab3_label = lv_label_create(tab3);
    lv_label_set_text(tab3_label, "Drift monitor:");
    lv_obj_align(tab3_label, LV_ALIGN_TOP_LEFT, 5, 5);

    label_drift = lv_label_create(tab3);
    lv_obj_align(label_drift, LV_ALIGN_TOP_LEFT, 5, 25);
    lv_label_set_text(label_drift, "Calculating...");
    lv_timer_create(lv_drift_timer_cb, 500, NULL);

    lv_obj_t* slider_tab4 = lv_slider_create(tab4);
    lv_obj_set_width(slider_tab4, 200);
    lv_obj_center(slider_tab4);
    lv_obj_add_event_cb(slider_tab4, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    slider_tab4_label = lv_label_create(tab4);
    lv_label_set_text(slider_tab4_label, "0");
    lv_obj_align_to(slider_tab4_label, slider_tab4, LV_ALIGN_OUT_TOP_MID, 0, -15);
}
