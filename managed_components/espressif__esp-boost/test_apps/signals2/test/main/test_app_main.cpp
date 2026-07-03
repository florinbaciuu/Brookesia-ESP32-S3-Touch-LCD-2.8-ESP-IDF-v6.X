/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "unity.h"
#include "unity_test_utils.h"
#include "common_components.hpp"

// Some resources are lazy allocated in the LCD driver, the threadhold is left for that case
#define TEST_MEMORY_LEAK_THRESHOLD (0)

void setUp(void)
{
    unity_utils_record_free_mem();
}

void tearDown(void)
{
    size_t memory_leak_threshold = common_get_memory_leak_threshold();
    esp_reent_cleanup();
    unity_utils_evaluate_leaks_direct(memory_leak_threshold);

    if (memory_leak_threshold != TEST_MEMORY_LEAK_THRESHOLD) {
        common_set_memory_leak_threshold(TEST_MEMORY_LEAK_THRESHOLD);
    }
}

extern "C" void app_main(void)
{
    /**
     *   ______   __                                __             ______        ________                      __
     *  /      \ |  \                              |  \           /      \      |        \                    |  \
     * |  $$$$$$\ \$$  ______   _______    ______  | $$  _______ |  $$$$$$\      \$$$$$$$$______    _______  _| $$_
     * | $$___\$$|  \ /      \ |       \  |      \ | $$ /       \ \$$__| $$ ______ | $$  /      \  /       \|   $$ \
     *  \$$    \ | $$|  $$$$$$\| $$$$$$$\  \$$$$$$\| $$|  $$$$$$$ /      $$|      \| $$ |  $$$$$$\|  $$$$$$$ \$$$$$$
     *  _\$$$$$$\| $$| $$  | $$| $$  | $$ /      $$| $$ \$$    \ |  $$$$$$  \$$$$$$| $$ | $$    $$ \$$    \   | $$ __
     * |  \__| $$| $$| $$__| $$| $$  | $$|  $$$$$$$| $$ _\$$$$$$\| $$_____         | $$ | $$$$$$$$ _\$$$$$$\  | $$|  \
     *  \$$    $$| $$ \$$    $$| $$  | $$ \$$    $$| $$|       $$| $$     \        | $$  \$$     \|       $$   \$$  $$
     *   \$$$$$$  \$$ _\$$$$$$$ \$$   \$$  \$$$$$$$ \$$ \$$$$$$$  \$$$$$$$$         \$$   \$$$$$$$ \$$$$$$$     \$$$$
     *               |  \__| $$
     *                \$$    $$
     *                 \$$$$$$
     */
    printf("  ______   __                                __             ______        ________                      __\r\n");
    printf(" /      \\ |  \\                              |  \\           /      \\      |        \\                    |  \\\r\n");
    printf("|  $$$$$$\\ \\$$  ______   _______    ______  | $$  _______ |  $$$$$$\\      \\$$$$$$$$______    _______  _| $$_\r\n");
    printf("| $$___\\$$|  \\ /      \\ |       \\  |      \\ | $$ /       \\ \\$$__| $$ ______ | $$  /      \\  /       \\|   $$ \\\r\n");
    printf(" \\$$    \\ | $$|  $$$$$$\\| $$$$$$$\\  \\$$$$$$\\| $$|  $$$$$$$ /      $$|      \\| $$ |  $$$$$$\\|  $$$$$$$ \\$$$$$$\r\n");
    printf(" _\\$$$$$$\\| $$| $$  | $$| $$  | $$ /      $$| $$ \\$$    \\ |  $$$$$$  \\$$$$$$| $$ | $$    $$ \\$$    \\   | $$ __\r\n");
    printf("|  \\__| $$| $$| $$__| $$| $$  | $$|  $$$$$$$| $$ _\\$$$$$$\\| $$_____         | $$ | $$$$$$$$ _\\$$$$$$\\  | $$|  \\\r\n");
    printf(" \\$$    $$| $$ \\$$    $$| $$  | $$ \\$$    $$| $$|       $$| $$     \\        | $$  \\$$     \\|       $$   \\$$  $$\r\n");
    printf("  \\$$$$$$  \\$$ _\\$$$$$$$ \\$$   \\$$  \\$$$$$$$ \\$$ \\$$$$$$$  \\$$$$$$$$         \\$$   \\$$$$$$$ \\$$$$$$$     \\$$$$\r\n");
    printf("              |  \\__| $$\r\n");
    printf("               \\$$    $$\r\n");
    printf("                \\$$$$$$\r\n");
    unity_run_menu();
}
