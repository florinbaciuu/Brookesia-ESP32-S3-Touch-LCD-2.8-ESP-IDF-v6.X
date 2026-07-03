#include "qmi8658.h"

#include <string.h>

#include "aux_i2c_bsp_interface.h"
#include "board_config.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define QMI8658_ADDR_LOW 0x6b
#define QMI8658_ADDR_HIGH 0x6a

#define QMI8658_WHO_AM_I 0x00
#define QMI8658_REVISION_ID 0x01
#define QMI8658_CTRL1 0x02
#define QMI8658_CTRL2 0x03
#define QMI8658_CTRL3 0x04
#define QMI8658_CTRL5 0x06
#define QMI8658_CTRL6 0x07
#define QMI8658_CTRL7 0x08
#define QMI8658_AX_L 0x35

#define QMI8658_CTRL1_AUTO_INC 0x40
#define QMI8658_CTRL7_ACC_GYRO_ENABLE 0x43

static const char* TAG = "QMI8658";

static i2c_master_dev_handle_t s_imu_dev = NULL;
static uint8_t s_address = 0;
static uint8_t s_who_am_i = 0;
static uint8_t s_revision = 0;
static bool s_ready = false;

static const float ACCEL_SCALE_G = 4.0f / 32768.0f;
static const float GYRO_SCALE_DPS = 64.0f / 32768.0f;

static esp_err_t add_device(uint8_t address, i2c_master_dev_handle_t* out_dev)
{
    const i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = I2C_AUX_MASTER_FREQ_HZ,
    };

    return i2c_master_bus_add_device(aux_i2c_bus, &dev_cfg, out_dev);
}

static esp_err_t read_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* data, size_t length)
{
    return i2c_master_transmit_receive(
        dev, &reg, sizeof(reg), data, length, pdMS_TO_TICKS(I2C_AUX_MASTER_TIMEOUT_MS));
}

static esp_err_t write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buffer[2] = { reg, value };
    return i2c_master_transmit(s_imu_dev, buffer, sizeof(buffer), pdMS_TO_TICKS(I2C_AUX_MASTER_TIMEOUT_MS));
}

static int16_t le_i16(const uint8_t* data)
{
    return (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

esp_err_t qmi8658_init(void)
{
    if (s_ready) {
        return ESP_OK;
    }

    bsp_aux_i2c_init();
    if (aux_i2c_bus == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    const uint8_t addresses[] = { QMI8658_ADDR_LOW, QMI8658_ADDR_HIGH };
    esp_err_t last_err = ESP_ERR_NOT_FOUND;

    for (size_t index = 0; index < sizeof(addresses); ++index) {
        i2c_master_dev_handle_t candidate = NULL;
        const uint8_t address = addresses[index];
        last_err = add_device(address, &candidate);
        if (last_err != ESP_OK) {
            continue;
        }

        uint8_t who_am_i = 0;
        uint8_t revision = 0;
        last_err = read_reg(candidate, QMI8658_WHO_AM_I, &who_am_i, 1);
        if (last_err == ESP_OK) {
            (void)read_reg(candidate, QMI8658_REVISION_ID, &revision, 1);
            s_imu_dev = candidate;
            s_address = address;
            s_who_am_i = who_am_i;
            s_revision = revision;
            break;
        }

        (void)i2c_master_bus_rm_device(candidate);
    }

    if (s_imu_dev == NULL) {
        ESP_LOGW(TAG, "QMI8658 not responding on auxiliary I2C bus: %s", esp_err_to_name(last_err));
        return last_err;
    }

    esp_err_t err = write_reg(QMI8658_CTRL1, QMI8658_CTRL1_AUTO_INC);
    if (err == ESP_OK) {
        err = write_reg(QMI8658_CTRL7, QMI8658_CTRL7_ACC_GYRO_ENABLE);
    }
    if (err == ESP_OK) {
        err = write_reg(QMI8658_CTRL6, 0x00);
    }
    if (err == ESP_OK) {
        err = write_reg(QMI8658_CTRL2, (1u << 4) | 0x00); // 4G, 8000Hz
    }
    if (err == ESP_OK) {
        err = write_reg(QMI8658_CTRL3, (2u << 4) | 0x00); // 64 dps, 8000Hz
    }
    if (err == ESP_OK) {
        err = write_reg(QMI8658_CTRL5, 0x77); // accel + gyro low-pass filters
    }
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "QMI8658 configuration failed: %s", esp_err_to_name(err));
        return err;
    }

    s_ready = true;
    ESP_LOGI(TAG,
        "QMI8658 ready on I2C address 0x%02x who=0x%02x rev=0x%02x",
        s_address,
        s_who_am_i,
        s_revision);
    return ESP_OK;
}

esp_err_t qmi8658_read(qmi8658_sample_t* sample)
{
    if (sample == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_err_t init_err = qmi8658_init();
    if (init_err != ESP_OK) {
        return init_err;
    }

    uint8_t buffer[12] = { 0 };
    const esp_err_t err = read_reg(s_imu_dev, QMI8658_AX_L, buffer, sizeof(buffer));
    if (err != ESP_OK) {
        return err;
    }

    memset(sample, 0, sizeof(*sample));
    sample->address = s_address;
    sample->who_am_i = s_who_am_i;
    sample->revision = s_revision;
    sample->accel_g.x = (float)le_i16(&buffer[0]) * ACCEL_SCALE_G;
    sample->accel_g.y = (float)le_i16(&buffer[2]) * ACCEL_SCALE_G;
    sample->accel_g.z = (float)le_i16(&buffer[4]) * ACCEL_SCALE_G;
    sample->gyro_dps.x = (float)le_i16(&buffer[6]) * GYRO_SCALE_DPS;
    sample->gyro_dps.y = (float)le_i16(&buffer[8]) * GYRO_SCALE_DPS;
    sample->gyro_dps.z = (float)le_i16(&buffer[10]) * GYRO_SCALE_DPS;
    return ESP_OK;
}

bool qmi8658_is_ready(void)
{
    return s_ready;
}
