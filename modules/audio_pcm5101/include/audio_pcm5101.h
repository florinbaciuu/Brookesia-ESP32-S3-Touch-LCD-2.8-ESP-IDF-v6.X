#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool ready;
    uint32_t sample_rate_hz;
    uint8_t volume_percent;
} audio_pcm5101_status_t;

esp_err_t audio_pcm5101_init(void);
esp_err_t audio_pcm5101_write(const int16_t* samples, size_t frame_count, uint32_t timeout_ms);
esp_err_t audio_pcm5101_play_test_tone(uint32_t duration_ms, uint32_t frequency_hz);
esp_err_t audio_pcm5101_set_volume(uint8_t volume_percent);
esp_err_t audio_pcm5101_get_status(audio_pcm5101_status_t* status);
bool audio_pcm5101_is_ready(void);

#ifdef __cplusplus
}
#endif
