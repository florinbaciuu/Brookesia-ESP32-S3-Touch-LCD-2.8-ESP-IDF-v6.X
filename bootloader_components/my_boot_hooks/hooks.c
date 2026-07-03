#include "esp_log.h"
#include "esp_log_level.h"

/* Function used to tell the linker to include this file
 * with all its symbols.
 */
void bootloader_hooks_include(void){
}


void bootloader_before_init(void) {
    /* Keep in my mind that a lot of functions cannot be called from here
     * as system initialization has not been performed yet, including
     * BSS, SPI flash, or memory protection. */
    ESP_EARLY_LOGI("HOOK", "This hook is called BEFORE bootloader initialization (ESP_EARLY_LOGI)");
    ESP_DRAM_LOGI("HOOK", "This hook is called BEFORE bootloader initialization (ESP_DRAM_LOGI)");
}

void bootloader_after_init(void) {
    ESP_EARLY_LOGI("HOOK", "This hook is called AFTER bootloader initialization (ESP_EARLY_LOGI)");
    ESP_DRAM_LOGI("HOOK", "This hook is called AFTER bootloader initialization (ESP_DRAM_LOGI)");
}
