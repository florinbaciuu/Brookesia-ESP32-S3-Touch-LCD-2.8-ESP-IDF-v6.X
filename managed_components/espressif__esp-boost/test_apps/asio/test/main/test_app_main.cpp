/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <inttypes.h>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "unity.h"
#include "unity_test_utils.h"
#include "common_components.hpp"

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

static void asio_test_init_lwip_stack(void)
{
    /* Asio TCP uses BSD sockets -> lwIP tcpip thread; must init before open()/resolve(). */
    esp_err_t err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        printf("esp_netif_init failed: %s\\n", esp_err_to_name(err));
        abort();
    }
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        printf("esp_event_loop_create_default failed: %s\\n", esp_err_to_name(err));
        abort();
    }
}

extern "C" void app_main(void)
{
    common_set_memory_leak_threshold(TEST_MEMORY_LEAK_THRESHOLD);
    asio_test_init_lwip_stack();

    /**
     *   ______    ______   ______   ______        ________                      __
     *  /      \  /      \ |      \ /      \      |        \                    |  \
     * |  $$$$$$\|  $$$$$$\ \$$$$$$|  $$$$$$\      \$$$$$$$$______    _______  _| $$_
     * | $$__| $$| $$___\$$  | $$  | $$  | $$ ______ | $$  /      \  /       \|   $$ \
     * | $$    $$ \$$    \   | $$  | $$  | $$|      \| $$ |  $$$$$$\|  $$$$$$$ \$$$$$$
     * | $$$$$$$$ _\$$$$$$\  | $$  | $$  | $$ \$$$$$$| $$ | $$    $$ \$$    \   | $$ __
     * | $$  | $$|  \__| $$ _| $$_ | $$__/ $$        | $$ | $$$$$$$$ _\$$$$$$\  | $$|  \
     * | $$  | $$ \$$    $$|   $$ \ \$$    $$        | $$  \$$     \|       $$   \$$  $$
     *  \$$   \$$  \$$$$$$  \$$$$$$  \$$$$$$          \$$   \$$$$$$$ \$$$$$$$     \$$$$
     */
    printf("  ______    ______   ______   ______        ________                      __\r\n");
    printf(" /      \\  /      \\ |      \\ /      \\      |        \\                    |  \\\r\n");
    printf("|  $$$$$$\\|  $$$$$$\\ \\$$$$$$|  $$$$$$\\      \\$$$$$$$$______    _______  _| $$_\r\n");
    printf("| $$__| $$| $$___\\$$  | $$  | $$  | $$ ______ | $$  /      \\  /       \\|   $$ \\\r\n");
    printf("| $$    $$ \\$$    \\   | $$  | $$  | $$|      \\| $$ |  $$$$$$\\|  $$$$$$$ \\$$$$$$\r\n");
    printf("| $$$$$$$$ _\\$$$$$$\\  | $$  | $$  | $$ \\$$$$$$| $$ | $$    $$ \\$$    \\   | $$ __\r\n");
    printf("| $$  | $$|  \\__| $$ _| $$_ | $$__/ $$        | $$ | $$$$$$$$ _\\$$$$$$\\  | $$|  \\\r\n");
    printf("| $$  | $$ \\$$    $$|   $$ \\ \\$$    $$        | $$  \\$$     \\|       $$   \\$$  $$\r\n");
    printf(" \\$$   \\$$  \\$$$$$$  \\$$$$$$  \\$$$$$$          \\$$   \\$$$$$$$ \\$$$$$$$     \\$$$$\r\n");
    unity_run_menu();
}
