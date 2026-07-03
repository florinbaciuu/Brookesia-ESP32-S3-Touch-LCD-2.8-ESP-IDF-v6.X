#include "brookesia_project_board.h"

#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "board_pins.h"
#include "audio_pcm5101.h"
#include "defines.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_littlefs.h"
#include "esp_spiffs.h"
#include "esp_vfs_fat.h"
#include "lcd_backlight.h"
#include "lcd_bsp_interface.h"
#include "power_key.h"
#include "qmi8658.h"
#include "rtc_pcf85063.h"
#include "touch_bsp_interface.h"
}

#include "brookesia/hal_interface.hpp"

using namespace esp_brookesia;

namespace {

constexpr const char* TAG = "BROOKESIA_BOARD";
constexpr int BATTERY_ADC_GPIO = 8;
constexpr float BATTERY_ADC_DIVIDER = 3.0f;
constexpr float BATTERY_MEASUREMENT_OFFSET = 0.990476f;

class ProjectBoardInfoIface : public hal::BoardInfoIface {
public:
    static constexpr const char* NAME = "BrookesiaProjectBoard:BoardInfo";

    ProjectBoardInfoIface()
        : hal::BoardInfoIface(hal::BoardInfoIface::Info {
              .name = "ESP32-S3 Touch LCD 2.8",
              .chip = CONFIG_IDF_TARGET,
              .version = "project-bsp",
              .description = "Project BSP bridge for ESP-Brookesia",
              .manufacturer = "Florin Baciu / Waveshare",
          })
    {
    }
};

class ProjectDisplayPanelIface : public hal::DisplayPanelIface {
public:
    static constexpr const char* NAME = "BrookesiaProjectBoard:DisplayPanel";

    ProjectDisplayPanelIface()
        : hal::DisplayPanelIface(hal::DisplayPanelIface::Info {
              .h_res = LCD_WIDTH,
              .v_res = LCD_HEIGHT,
              .pixel_format = hal::DisplayPanelIface::PixelFormat::RGB565,
          })
    {
    }

    bool draw_bitmap(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, const uint8_t* data) override
    {
        if (panel_handle == nullptr || data == nullptr || x2 <= x1 || y2 <= y1) {
            return false;
        }

        return esp_lcd_panel_draw_bitmap(panel_handle, x1, y1, x2, y2, data) == ESP_OK;
    }

    bool get_driver_specific(DriverSpecific& specific) override
    {
        specific.panel_handle = reinterpret_cast<void*>(panel_handle);
        specific.bus_type = hal::DisplayPanelIface::BusType::Generic;
        specific.draw_x_align_bytes = 1;
        specific.draw_y_align_bytes = 1;
        return panel_handle != nullptr;
    }
};

class ProjectDisplayBacklightIface : public hal::DisplayBacklightIface {
public:
    static constexpr const char* NAME = "BrookesiaProjectBoard:DisplayBacklight";

    bool set_brightness(uint8_t percent) override
    {
        Backlight_Set(percent);
        return true;
    }

    bool get_brightness(uint8_t& percent) override
    {
        percent = s_backlight_level;
        return true;
    }

    bool is_light_on_off_supported() override
    {
        return true;
    }

    bool set_light_on_off(bool on) override
    {
        Backlight_Set(on ? s_backlight_level : 0);
        light_on_ = on;
        return true;
    }

    bool is_light_on() const override
    {
        return light_on_;
    }

private:
    bool light_on_ = true;
};

class ProjectDisplayTouchIface : public hal::DisplayTouchIface {
public:
    static constexpr const char* NAME = "BrookesiaProjectBoard:DisplayTouch";

    ProjectDisplayTouchIface()
        : hal::DisplayTouchIface(hal::DisplayTouchIface::Info {
              .x_max = LCD_WIDTH,
              .y_max = LCD_HEIGHT,
              .operation_mode = hal::DisplayTouchIface::OperationMode::Polling,
          })
    {
    }

    bool read_points(std::vector<Point>& points) override
    {
        uint16_t x = 0;
        uint16_t y = 0;

        points.clear();
        if (touch_read(&x, &y)) {
            points.push_back(Point {
                .x = static_cast<int16_t>(x),
                .y = static_cast<int16_t>(y),
                .pressure = 1,
            });
        }
        return true;
    }

    bool register_interrupt_handler(InterruptHandler handler) override
    {
        interrupt_handler_ = std::move(handler);
        return false;
    }
};

class ProjectStorageFsIface : public hal::StorageFsIface {
public:
    static constexpr const char* NAME = "BrookesiaProjectBoard:StorageFs";

    ProjectStorageFsIface()
        : hal::StorageFsIface()
    {
        add_if_mounted(
            hal::StorageFsIface::FileSystemType::FATFS, hal::StorageFsIface::MediumType::SDCard, SD_MOUNT_PATH, true);
        add_if_mounted(
            hal::StorageFsIface::FileSystemType::FATFS, hal::StorageFsIface::MediumType::Flash, FAT_MOUNT_PATH, true);
        add_if_mounted(
            hal::StorageFsIface::FileSystemType::SPIFFS, hal::StorageFsIface::MediumType::Flash, SPIFFS_MOUNT_PATH, false);
        add_if_mounted(
            hal::StorageFsIface::FileSystemType::LittleFS, hal::StorageFsIface::MediumType::Flash, LITTLEFS_MOUNT_PATH, true);
    }

    bool get_capacity(const char* mount_point, Capacity& capacity) override
    {
        if (mount_point == nullptr) {
            return false;
        }

        uint64_t total_bytes = 0;
        uint64_t free_bytes = 0;
        esp_err_t err = ESP_ERR_NOT_SUPPORTED;

        if (strcmp(mount_point, SD_MOUNT_PATH) == 0 || strcmp(mount_point, FAT_MOUNT_PATH) == 0) {
            err = esp_vfs_fat_info(mount_point, &total_bytes, &free_bytes);
        } else if (strcmp(mount_point, SPIFFS_MOUNT_PATH) == 0) {
            size_t total = 0;
            size_t used = 0;
            err = esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used);
            total_bytes = total;
            free_bytes = total >= used ? total - used : 0;
        } else if (strcmp(mount_point, LITTLEFS_MOUNT_PATH) == 0) {
            size_t total = 0;
            size_t used = 0;
            err = esp_littlefs_info(LITTLEFS_PARTITION_LABEL, &total, &used);
            total_bytes = total;
            free_bytes = total >= used ? total - used : 0;
        }

        if (err != ESP_OK) {
            ESP_LOGD(TAG, "Storage capacity unavailable for %s: %s", mount_point, esp_err_to_name(err));
            return false;
        }

        capacity.total_bytes = total_bytes;
        capacity.free_bytes = free_bytes;
        capacity.used_bytes = capacity.total_bytes - capacity.free_bytes;
        return true;
    }

private:
    void add_if_mounted(FileSystemType fs_type, MediumType medium_type, const char* mount_point, bool supports_directories)
    {
        Capacity capacity = {};
        if (!get_capacity(mount_point, capacity)) {
            ESP_LOGI(TAG, "Storage mount point not active: %s", mount_point);
            return;
        }

        info_list_.push_back(Info {
            .fs_type = fs_type,
            .medium_type = medium_type,
            .mount_point = mount_point,
            .supports_directories = supports_directories,
        });
        ESP_LOGI(TAG,
            "Storage mount point active: %s total=%llu used=%llu free=%llu",
            mount_point,
            capacity.total_bytes,
            capacity.used_bytes,
            capacity.free_bytes);
    }
};

class ProjectPowerBatteryIface : public hal::PowerBatteryIface {
public:
    static constexpr const char* NAME = "BrookesiaProjectBoard:PowerBattery";

    ProjectPowerBatteryIface()
        : hal::PowerBatteryIface(hal::PowerBatteryIface::Info {
              .name = "ESP32-S3-Touch-LCD-2.8 battery",
              .chemistry = "Li-ion",
              .abilities = {
#if CONFIG_BROOKESIA_ADAPTER_POWER_BATTERY_ADC
                  hal::PowerBatteryIface::Ability::Voltage,
                  hal::PowerBatteryIface::Ability::Percentage,
#else
                  hal::PowerBatteryIface::Ability::PowerSource,
#endif
              },
          })
    {
#if CONFIG_BROOKESIA_ADAPTER_POWER_BATTERY_ADC
        init_adc();
#endif
    }

    bool get_state(State& state) override
    {
        uint32_t battery_mv = 0;
        const bool has_battery_voltage = read_battery_mv(battery_mv);
        const bool battery_present = has_battery_voltage && battery_mv >= 3000;

        state = State {
            .is_present = battery_present,
            .power_source = battery_present ? PowerSource::Unknown : PowerSource::External,
            .charge_state = battery_present ? ChargeState::Unknown : ChargeState::NotCharging,
            .level_source = has_battery_voltage ? LevelSource::VoltageCurve : LevelSource::Unknown,
            .voltage_mv = has_battery_voltage ? std::optional<uint32_t>(battery_mv) : std::nullopt,
            .percentage = has_battery_voltage ? std::optional<uint8_t>(estimate_percentage(battery_mv)) : std::nullopt,
            .vbus_voltage_mv = std::nullopt,
            .system_voltage_mv = std::nullopt,
            .is_low = has_battery_voltage && battery_mv < 3500,
            .is_critical = has_battery_voltage && battery_mv < 3300,
        };
        return true;
    }

    bool get_charge_config(ChargeConfig& config) override
    {
        config = ChargeConfig {};
        return false;
    }

    bool set_charge_config(const ChargeConfig& config) override
    {
        (void) config;
        return false;
    }

    bool set_charging_enabled(bool enabled) override
    {
        (void) enabled;
        return false;
    }

private:
    void init_adc()
    {
#if CONFIG_BROOKESIA_ADAPTER_POWER_BATTERY_ADC
        if (adc_ready_) {
            return;
        }

        adc_unit_t unit = ADC_UNIT_1;
        adc_channel_t channel = ADC_CHANNEL_0;
        if (adc_oneshot_io_to_channel(BATTERY_ADC_GPIO, &unit, &channel) != ESP_OK) {
            ESP_LOGW(TAG, "Battery ADC GPIO%d is not ADC capable", BATTERY_ADC_GPIO);
            return;
        }

        adc_oneshot_unit_init_cfg_t unit_cfg = {
            .unit_id = unit,
            .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };
        if (adc_oneshot_new_unit(&unit_cfg, &adc_handle_) != ESP_OK) {
            ESP_LOGW(TAG, "Failed to create battery ADC unit");
            return;
        }

        battery_adc_channel_ = channel;
        adc_oneshot_chan_cfg_t channel_cfg = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };
        if (adc_oneshot_config_channel(adc_handle_, battery_adc_channel_, &channel_cfg) != ESP_OK) {
            ESP_LOGW(TAG, "Failed to configure battery ADC channel");
            return;
        }

        adc_cali_curve_fitting_config_t cali_cfg = {
            .unit_id = unit,
            .chan = battery_adc_channel_,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };
        if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &adc_cali_handle_) != ESP_OK) {
            ESP_LOGW(TAG, "Battery ADC calibration unavailable; voltage reporting disabled");
            return;
        }

        adc_ready_ = true;
        ESP_LOGI(TAG, "Battery ADC ready on GPIO%d", BATTERY_ADC_GPIO);
#endif
    }

    bool read_battery_mv(uint32_t& battery_mv)
    {
#if CONFIG_BROOKESIA_ADAPTER_POWER_BATTERY_ADC
        if (!adc_ready_) {
            return false;
        }

        int adc_mv = 0;
        if (adc_oneshot_get_calibrated_result(adc_handle_, adc_cali_handle_, battery_adc_channel_, &adc_mv) != ESP_OK) {
            return false;
        }

        const float corrected_mv = (static_cast<float>(adc_mv) * BATTERY_ADC_DIVIDER) / BATTERY_MEASUREMENT_OFFSET;
        battery_mv = static_cast<uint32_t>(corrected_mv + 0.5f);
        return true;
#else
        (void) battery_mv;
        return false;
#endif
    }

    static uint8_t estimate_percentage(uint32_t battery_mv)
    {
        if (battery_mv <= 3300) {
            return 0;
        }
        if (battery_mv >= 4200) {
            return 100;
        }
        return static_cast<uint8_t>(((battery_mv - 3300) * 100) / (4200 - 3300));
    }

    adc_oneshot_unit_handle_t adc_handle_ = nullptr;
    adc_cali_handle_t adc_cali_handle_ = nullptr;
    adc_channel_t battery_adc_channel_ = ADC_CHANNEL_0;
    bool adc_ready_ = false;
};

class ProjectBoardDevice : public hal::Device {
public:
    static constexpr const char* NAME = "BrookesiaProjectBoard";

    ProjectBoardDevice()
        : hal::Device(std::string(NAME))
    {
    }

    bool probe() override
    {
        return panel_handle != nullptr;
    }

    bool on_init() override
    {
#if CONFIG_BROOKESIA_ADAPTER_RTC_PCF85063_ENABLE
        const esp_err_t rtc_err = rtc_pcf85063_init();
        if (rtc_err != ESP_OK) {
            ESP_LOGW(TAG, "PCF85063 RTC unavailable: %s", esp_err_to_name(rtc_err));
        } else {
            rtc_pcf85063_datetime_t rtc_time = {};
            const esp_err_t read_err = rtc_pcf85063_read(&rtc_time);
            if (read_err == ESP_OK && !rtc_pcf85063_datetime_is_valid(&rtc_time)) {
#if CONFIG_BROOKESIA_ADAPTER_RTC_SET_INVALID_FROM_BUILD_TIME
                const esp_err_t set_err = rtc_pcf85063_set_build_time();
                if (set_err != ESP_OK) {
                    ESP_LOGW(TAG, "Failed to set invalid PCF85063 RTC from build time: %s", esp_err_to_name(set_err));
                }
#else
                ESP_LOGW(TAG, "PCF85063 RTC date is invalid");
#endif
            }
        }
#endif
#if CONFIG_BROOKESIA_ADAPTER_IMU_QMI8658_ENABLE
        const esp_err_t imu_err = qmi8658_init();
        if (imu_err != ESP_OK) {
            ESP_LOGW(TAG, "QMI8658 IMU unavailable: %s", esp_err_to_name(imu_err));
        }
#endif
#if CONFIG_BROOKESIA_ADAPTER_POWER_KEY_ENABLE
        const esp_err_t power_key_err = power_key_init();
        if (power_key_err != ESP_OK) {
            ESP_LOGW(TAG, "Power key service unavailable: %s", esp_err_to_name(power_key_err));
        }
#endif
#if CONFIG_BROOKESIA_ADAPTER_AUDIO_PCM5101_ENABLE
        const esp_err_t audio_err = audio_pcm5101_init();
        if (audio_err != ESP_OK) {
            ESP_LOGW(TAG, "PCM5101 audio unavailable: %s", esp_err_to_name(audio_err));
        }
#if CONFIG_BROOKESIA_ADAPTER_AUDIO_PCM5101_TEST_TONE
        else {
            const esp_err_t tone_err = audio_pcm5101_play_test_tone(250, 440);
            if (tone_err != ESP_OK) {
                ESP_LOGW(TAG, "PCM5101 test tone failed: %s", esp_err_to_name(tone_err));
            }
        }
#endif
#endif
        interfaces_.emplace(ProjectBoardInfoIface::NAME, std::make_shared<ProjectBoardInfoIface>());
        interfaces_.emplace(ProjectDisplayPanelIface::NAME, std::make_shared<ProjectDisplayPanelIface>());
        interfaces_.emplace(ProjectDisplayBacklightIface::NAME, std::make_shared<ProjectDisplayBacklightIface>());
        interfaces_.emplace(ProjectDisplayTouchIface::NAME, std::make_shared<ProjectDisplayTouchIface>());
        interfaces_.emplace(ProjectStorageFsIface::NAME, std::make_shared<ProjectStorageFsIface>());
        interfaces_.emplace(ProjectPowerBatteryIface::NAME, std::make_shared<ProjectPowerBatteryIface>());
        return true;
    }

    void on_deinit() override
    {
        interfaces_.clear();
    }
};

} // namespace

BROOKESIA_PLUGIN_REGISTER(hal::Device, ProjectBoardDevice, std::string(ProjectBoardDevice::NAME));

extern "C" bool brookesia_project_board_init(void)
{
    const bool initialized = hal::init_device(ProjectBoardDevice::NAME);
    if (!initialized) {
        ESP_LOGE(TAG, "Failed to initialize %s HAL device", ProjectBoardDevice::NAME);
        return false;
    }

    const auto capabilities = hal::get_capabilities();
    for (const auto& [type, names] : capabilities) {
        ESP_LOGI(TAG, "HAL capability %s: %u instance(s)", type.c_str(), static_cast<unsigned>(names.size()));
    }
    return true;
}

extern "C" void brookesia_project_board_deinit(void)
{
    hal::deinit_device(ProjectBoardDevice::NAME);
}

extern "C" void brookesia_project_board_get_status(char* buffer, int buffer_size)
{
    if (buffer == nullptr || buffer_size <= 0) {
        return;
    }

    const auto capabilities = hal::get_capabilities();
    auto [storage_name, storage_iface] = hal::get_first_interface<hal::StorageFsIface>();
    auto [battery_name, battery_iface] = hal::get_first_interface<hal::PowerBatteryIface>();
    const unsigned storage_count = storage_iface ? static_cast<unsigned>(storage_iface->get_all_info().size()) : 0;
    hal::PowerBatteryIface::State battery_state = {};
    const bool battery_ok = battery_iface && battery_iface->get_state(battery_state);
    char power_status[40] = "unknown";
    if (battery_ok) {
        if (battery_state.voltage_mv.has_value() && battery_state.percentage.has_value()) {
            snprintf(power_status,
                sizeof(power_status),
                "%lumV %u%%",
                static_cast<unsigned long>(battery_state.voltage_mv.value()),
                static_cast<unsigned>(battery_state.percentage.value()));
        } else if (battery_state.power_source == hal::PowerBatteryIface::PowerSource::External) {
            snprintf(power_status, sizeof(power_status), "external");
        } else {
            snprintf(power_status, sizeof(power_status), "unknown");
        }
    }

    char rtc_status[32] = "disabled";
#if CONFIG_BROOKESIA_ADAPTER_RTC_PCF85063_ENABLE
    snprintf(rtc_status, sizeof(rtc_status), "unavailable");
    rtc_pcf85063_datetime_t rtc_time = {};
    if (rtc_pcf85063_read(&rtc_time) == ESP_OK) {
        rtc_pcf85063_format(&rtc_time, rtc_status, sizeof(rtc_status));
    }
#endif

    char imu_status[96] = "disabled";
#if CONFIG_BROOKESIA_ADAPTER_IMU_QMI8658_ENABLE
    snprintf(imu_status, sizeof(imu_status), "unavailable");
    qmi8658_sample_t imu_sample = {};
    if (qmi8658_read(&imu_sample) == ESP_OK) {
        snprintf(imu_status,
            sizeof(imu_status),
            "A %.2f %.2f %.2fg / G %.1f %.1f %.1fdps",
            static_cast<double>(imu_sample.accel_g.x),
            static_cast<double>(imu_sample.accel_g.y),
            static_cast<double>(imu_sample.accel_g.z),
            static_cast<double>(imu_sample.gyro_dps.x),
            static_cast<double>(imu_sample.gyro_dps.y),
            static_cast<double>(imu_sample.gyro_dps.z));
    }
#endif

    char power_key_status[40] = "disabled";
#if CONFIG_BROOKESIA_ADAPTER_POWER_KEY_ENABLE
    snprintf(power_key_status, sizeof(power_key_status), "unavailable");
    power_key_status_t key_status = {};
    if (power_key_get_status(&key_status) == ESP_OK) {
        snprintf(power_key_status,
            sizeof(power_key_status),
            "%s, latch %s",
            key_status.key_pressed ? "pressed" : "released",
            key_status.latch_enabled ? "on" : "off");
    }
#endif

    char audio_status[48] = "disabled";
#if CONFIG_BROOKESIA_ADAPTER_AUDIO_PCM5101_ENABLE
    snprintf(audio_status, sizeof(audio_status), "unavailable");
    audio_pcm5101_status_t audio = {};
    if (audio_pcm5101_get_status(&audio) == ESP_OK) {
        snprintf(audio_status,
            sizeof(audio_status),
            "%s %luHz vol=%u%%",
            audio.ready ? "ready" : "idle",
            static_cast<unsigned long>(audio.sample_rate_hz),
            static_cast<unsigned>(audio.volume_percent));
    }
#endif

    snprintf(buffer,
        buffer_size,
        "HAL device: %s\nCapabilities: %u\nStorage mounts: %u\nPower: %s%s\nPWR key: %s\nAudio: %s\nRTC: %s\nIMU: %s\nPanel: %ux%u RGB565",
        ProjectBoardDevice::NAME,
        static_cast<unsigned>(capabilities.size()),
        storage_count,
        power_status,
        battery_ok && battery_state.is_present ? " + battery" : "",
        power_key_status,
        audio_status,
        rtc_status,
        imu_status,
        static_cast<unsigned>(LCD_WIDTH),
        static_cast<unsigned>(LCD_HEIGHT));
}
