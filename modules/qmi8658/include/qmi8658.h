#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x;
    float y;
    float z;
} qmi8658_vec3_t;

typedef struct {
    qmi8658_vec3_t accel_g;
    qmi8658_vec3_t gyro_dps;
    uint8_t address;
    uint8_t who_am_i;
    uint8_t revision;
} qmi8658_sample_t;

esp_err_t qmi8658_init(void);
esp_err_t qmi8658_read(qmi8658_sample_t* sample);
bool qmi8658_is_ready(void);

#ifdef __cplusplus
}
#endif
