#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} rtc_pcf85063_datetime_t;

esp_err_t rtc_pcf85063_init(void);
esp_err_t rtc_pcf85063_read(rtc_pcf85063_datetime_t* time);
esp_err_t rtc_pcf85063_set_datetime(const rtc_pcf85063_datetime_t* time);
esp_err_t rtc_pcf85063_set_build_time(void);
bool rtc_pcf85063_datetime_is_valid(const rtc_pcf85063_datetime_t* time);
int rtc_pcf85063_format(const rtc_pcf85063_datetime_t* time, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif
