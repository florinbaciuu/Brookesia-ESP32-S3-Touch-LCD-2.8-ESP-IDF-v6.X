#include "brookesia_project_board.h"

extern "C" {
#include "FAT_fs.h"
#include "LITTLE_fs.h"
#include "SPIF_fs.h"
#include "eMMC_fs.h"
#include "esp_err.h"
#include "esp_log.h"
}

namespace {

constexpr const char* TAG = "BROOKESIA_STORAGE";

void log_mount_result(const char* name, esp_err_t result)
{
    if (result == ESP_OK) {
        ESP_LOGI(TAG, "%s mounted", name);
        return;
    }

    ESP_LOGW(TAG, "%s mount skipped or failed: %s", name, esp_err_to_name(result));
}

} // namespace

extern "C" void brookesia_storage_automount_start(void)
{
#if CONFIG_BROOKESIA_ADAPTER_STORAGE_AUTOMOUNT_SDMMC
    log_mount_result("SD/MMC FAT", initialize_filesystem_sdmmc());
#endif

#if CONFIG_BROOKESIA_ADAPTER_STORAGE_AUTOMOUNT_FAT
    log_mount_result("Internal FAT", initialize_internal_fat_filesystem());
#endif

#if CONFIG_BROOKESIA_ADAPTER_STORAGE_AUTOMOUNT_SPIFFS
    log_mount_result("SPIFFS", initialize_filesystem_spiffs());
#endif

#if CONFIG_BROOKESIA_ADAPTER_STORAGE_AUTOMOUNT_LITTLEFS
    log_mount_result("LittleFS", initialize_filesystem_littlefs());
#endif

#if !CONFIG_BROOKESIA_ADAPTER_STORAGE_AUTOMOUNT_SDMMC && \
    !CONFIG_BROOKESIA_ADAPTER_STORAGE_AUTOMOUNT_FAT && \
    !CONFIG_BROOKESIA_ADAPTER_STORAGE_AUTOMOUNT_SPIFFS && \
    !CONFIG_BROOKESIA_ADAPTER_STORAGE_AUTOMOUNT_LITTLEFS
    ESP_LOGI(TAG, "Storage auto-mount disabled");
#endif
}
