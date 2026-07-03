#include "audio_pcm5101.h"

#include "board_config.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char* TAG = "AUDIO_PCM5101";

static i2s_chan_handle_t s_tx_chan = NULL;
static bool s_ready = false;
static uint32_t s_sample_rate_hz = AUDIO_PCM5101_SAMPLE_RATE_HZ;
static uint8_t s_volume_percent = 50;

esp_err_t audio_pcm5101_init(void)
{
    if (s_ready) {
        return ESP_OK;
    }

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(AUDIO_PCM5101_I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;

    esp_err_t err = i2s_new_channel(&chan_cfg, &s_tx_chan, NULL);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to create I2S channel: %s", esp_err_to_name(err));
        return err;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_PCM5101_SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_NC,
            .bclk = AUDIO_PCM5101_BCLK_GPIO,
            .ws = AUDIO_PCM5101_WS_GPIO,
            .dout = AUDIO_PCM5101_DOUT_GPIO,
            .din = GPIO_NUM_NC,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    err = i2s_channel_init_std_mode(s_tx_chan, &std_cfg);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to init I2S std mode: %s", esp_err_to_name(err));
        return err;
    }

    err = i2s_channel_enable(s_tx_chan);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to enable I2S channel: %s", esp_err_to_name(err));
        return err;
    }

    s_ready = true;
    ESP_LOGI(TAG,
        "PCM5101 ready on I2S%d BCLK=%d WS=%d DOUT=%d rate=%lu",
        AUDIO_PCM5101_I2S_NUM,
        AUDIO_PCM5101_BCLK_GPIO,
        AUDIO_PCM5101_WS_GPIO,
        AUDIO_PCM5101_DOUT_GPIO,
        (unsigned long)s_sample_rate_hz);
    return ESP_OK;
}

esp_err_t audio_pcm5101_write(const int16_t* samples, size_t frame_count, uint32_t timeout_ms)
{
    if (samples == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_err_t init_err = audio_pcm5101_init();
    if (init_err != ESP_OK) {
        return init_err;
    }

    size_t bytes_written = 0;
    const size_t byte_count = frame_count * 2 * sizeof(int16_t);
    return i2s_channel_write(s_tx_chan, samples, byte_count, &bytes_written, pdMS_TO_TICKS(timeout_ms));
}

esp_err_t audio_pcm5101_play_test_tone(uint32_t duration_ms, uint32_t frequency_hz)
{
    if (frequency_hz == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_err_t init_err = audio_pcm5101_init();
    if (init_err != ESP_OK) {
        return init_err;
    }

    const uint32_t total_frames = (s_sample_rate_hz * duration_ms) / 1000;
    const uint32_t half_period = (s_sample_rate_hz / frequency_hz) / 2;
    const int16_t amplitude = (int16_t)((12000 * s_volume_percent) / 100);
    int16_t buffer[256 * 2] = { 0 };
    uint32_t frame = 0;

    while (frame < total_frames) {
        const uint32_t frames_this_write = (total_frames - frame) > 256 ? 256 : (total_frames - frame);
        for (uint32_t index = 0; index < frames_this_write; ++index) {
            const bool high = half_period == 0 ? true : (((frame + index) / half_period) % 2) == 0;
            const int16_t value = high ? amplitude : (int16_t)-amplitude;
            buffer[index * 2] = value;
            buffer[index * 2 + 1] = value;
        }

        const esp_err_t err = audio_pcm5101_write(buffer, frames_this_write, 1000);
        if (err != ESP_OK) {
            return err;
        }
        frame += frames_this_write;
    }

    ESP_LOGI(TAG, "PCM5101 test tone played: %lums at %luHz", (unsigned long)duration_ms, (unsigned long)frequency_hz);
    return ESP_OK;
}

esp_err_t audio_pcm5101_set_volume(uint8_t volume_percent)
{
    if (volume_percent > 100) {
        return ESP_ERR_INVALID_ARG;
    }

    s_volume_percent = volume_percent;
    return ESP_OK;
}

esp_err_t audio_pcm5101_get_status(audio_pcm5101_status_t* status)
{
    if (status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    status->ready = s_ready;
    status->sample_rate_hz = s_sample_rate_hz;
    status->volume_percent = s_volume_percent;
    return ESP_OK;
}

bool audio_pcm5101_is_ready(void)
{
    return s_ready;
}
