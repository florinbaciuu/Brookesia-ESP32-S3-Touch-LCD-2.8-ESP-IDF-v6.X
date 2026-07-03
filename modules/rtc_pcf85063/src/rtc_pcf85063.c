#include "rtc_pcf85063.h"

#include <stdio.h>
#include <string.h>

#include "aux_i2c_bsp_interface.h"
#include "board_config.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define PCF85063_ADDRESS 0x51
#define PCF85063_REG_CTRL_1 0x00
#define PCF85063_REG_SECOND 0x04
#define PCF85063_CTRL_1_CAP_SEL 0x01
#define PCF85063_YEAR_OFFSET 1970

static const char* TAG = "RTC_PCF85063";

static i2c_master_dev_handle_t s_rtc_dev = NULL;
static bool s_initialized = false;

static uint8_t dec_to_bcd(uint8_t value)
{
    return (uint8_t)(((value / 10) << 4) | (value % 10));
}

static uint8_t bcd_to_dec(uint8_t value)
{
    return (uint8_t)(((value >> 4) * 10) + (value & 0x0f));
}

static esp_err_t write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buffer[2] = { reg, value };
    return i2c_master_transmit(s_rtc_dev, buffer, sizeof(buffer), pdMS_TO_TICKS(I2C_AUX_MASTER_TIMEOUT_MS));
}

static int month_from_build_string(const char* month)
{
    static const char* months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    for (int index = 0; index < 12; ++index) {
        if (strncmp(month, months[index], 3) == 0) {
            return index + 1;
        }
    }
    return 0;
}

static uint8_t weekday_from_date(uint16_t year, uint8_t month, uint8_t day)
{
    if (month < 3) {
        month += 12;
        year--;
    }

    const uint16_t k = year % 100;
    const uint16_t j = year / 100;
    const uint8_t h = (uint8_t)((day + ((13 * (month + 1)) / 5) + k + (k / 4) + (j / 4) + (5 * j)) % 7);
    return (uint8_t)((h + 6) % 7);
}

esp_err_t rtc_pcf85063_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    bsp_aux_i2c_init();
    if (aux_i2c_bus == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (s_rtc_dev == NULL) {
        const i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = PCF85063_ADDRESS,
            .scl_speed_hz = I2C_AUX_MASTER_FREQ_HZ,
        };

        const esp_err_t add_err = i2c_master_bus_add_device(aux_i2c_bus, &dev_cfg, &s_rtc_dev);
        if (add_err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to attach PCF85063 on I2C address 0x%02x: %s",
                PCF85063_ADDRESS,
                esp_err_to_name(add_err));
            return add_err;
        }
    }

    const esp_err_t err = write_reg(PCF85063_REG_CTRL_1, PCF85063_CTRL_1_CAP_SEL);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "PCF85063 not responding on I2C address 0x%02x: %s",
            PCF85063_ADDRESS,
            esp_err_to_name(err));
        return err;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "PCF85063 RTC ready on I2C address 0x%02x", PCF85063_ADDRESS);
    return ESP_OK;
}

esp_err_t rtc_pcf85063_set_datetime(const rtc_pcf85063_datetime_t* time)
{
    if (!rtc_pcf85063_datetime_is_valid(time)) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_err_t init_err = rtc_pcf85063_init();
    if (init_err != ESP_OK) {
        return init_err;
    }

    uint8_t buffer[8] = {
        PCF85063_REG_SECOND,
        dec_to_bcd(time->second),
        dec_to_bcd(time->minute),
        dec_to_bcd(time->hour),
        dec_to_bcd(time->day),
        dec_to_bcd(time->weekday),
        dec_to_bcd(time->month),
        dec_to_bcd((uint8_t)(time->year - PCF85063_YEAR_OFFSET)),
    };

    const esp_err_t err = i2c_master_transmit(s_rtc_dev, buffer, sizeof(buffer), pdMS_TO_TICKS(I2C_AUX_MASTER_TIMEOUT_MS));
    if (err == ESP_OK) {
        char formatted[32] = { 0 };
        rtc_pcf85063_format(time, formatted, sizeof(formatted));
        ESP_LOGI(TAG, "PCF85063 time set to %s", formatted);
    }
    return err;
}

esp_err_t rtc_pcf85063_set_build_time(void)
{
    const char* build_date = __DATE__;
    const char* build_time = __TIME__;
    char month_name[4] = { 0 };
    rtc_pcf85063_datetime_t time = {};

    int year = 0;
    int month_day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    if (sscanf(build_date, "%3s %d %d", month_name, &month_day, &year) != 3 ||
        sscanf(build_time, "%d:%d:%d", &hour, &minute, &second) != 3) {
        return ESP_ERR_INVALID_ARG;
    }

    const int month = month_from_build_string(month_name);
    if (month <= 0) {
        return ESP_ERR_INVALID_ARG;
    }

    time.year = (uint16_t)year;
    time.month = (uint8_t)month;
    time.day = (uint8_t)month_day;
    time.weekday = weekday_from_date(time.year, time.month, time.day);
    time.hour = (uint8_t)hour;
    time.minute = (uint8_t)minute;
    time.second = (uint8_t)second;

    return rtc_pcf85063_set_datetime(&time);
}

esp_err_t rtc_pcf85063_read(rtc_pcf85063_datetime_t* time)
{
    if (time == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_err_t init_err = rtc_pcf85063_init();
    if (init_err != ESP_OK) {
        return init_err;
    }

    uint8_t reg = PCF85063_REG_SECOND;
    uint8_t buffer[7] = { 0 };
    const esp_err_t err = i2c_master_transmit_receive(
        s_rtc_dev, &reg, sizeof(reg), buffer, sizeof(buffer), pdMS_TO_TICKS(I2C_AUX_MASTER_TIMEOUT_MS));
    if (err != ESP_OK) {
        return err;
    }

    time->second = bcd_to_dec(buffer[0] & 0x7f);
    time->minute = bcd_to_dec(buffer[1] & 0x7f);
    time->hour = bcd_to_dec(buffer[2] & 0x3f);
    time->day = bcd_to_dec(buffer[3] & 0x3f);
    time->weekday = bcd_to_dec(buffer[4] & 0x07);
    time->month = bcd_to_dec(buffer[5] & 0x1f);
    time->year = (uint16_t)(bcd_to_dec(buffer[6]) + PCF85063_YEAR_OFFSET);

    return ESP_OK;
}

bool rtc_pcf85063_datetime_is_valid(const rtc_pcf85063_datetime_t* time)
{
    if (time == NULL) {
        return false;
    }

    return time->year >= 2024 && time->year <= 2069 &&
           time->month >= 1 && time->month <= 12 &&
           time->day >= 1 && time->day <= 31 &&
           time->weekday <= 6 &&
           time->hour <= 23 &&
           time->minute <= 59 &&
           time->second <= 59;
}

int rtc_pcf85063_format(const rtc_pcf85063_datetime_t* time, char* buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return 0;
    }

    if (!rtc_pcf85063_datetime_is_valid(time)) {
        return snprintf(buffer, buffer_size, "invalid");
    }

    return snprintf(buffer,
        buffer_size,
        "%04u-%02u-%02u %02u:%02u:%02u",
        (unsigned)time->year,
        (unsigned)time->month,
        (unsigned)time->day,
        (unsigned)time->hour,
        (unsigned)time->minute,
        (unsigned)time->second);
}
