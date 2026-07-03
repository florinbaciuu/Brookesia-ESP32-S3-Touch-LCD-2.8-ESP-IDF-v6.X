#include "lvgl_framework.h"
#include "lvgl_framework_internals.h"

static SemaphoreHandle_t s_lvgl_mutex = NULL;

// -------------------------------

bool s_lvgl_port_init_locking_mutex(void) {
    if (!s_lvgl_mutex)
        s_lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    return (s_lvgl_mutex != NULL);
}

// -------------------------------
// -------------------------------

bool s_lvgl_lock(TickType_t timeout_ms) {
    if (!s_lvgl_mutex) {
        return false;
    }
    return (xSemaphoreTakeRecursive(s_lvgl_mutex, timeout_ms) == pdTRUE);
}

// -------------------------------

void s_lvgl_unlock(void) {
    if (s_lvgl_mutex)
        xSemaphoreGiveRecursive(s_lvgl_mutex);
}

// -------------------------------

bool lvgl_lock(int timeout_ms) {
    const TickType_t timeout_ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return s_lvgl_lock(timeout_ticks);
}

// -------------------------------

void lvgl_unlock(void) {
    s_lvgl_unlock();
}

// -------------------------------

/**
 * Executes a given function while holding the LVGL mutex lock to ensure thread safety when accessing LVGL functions.
 * The function will block until the lock is acquired, ensuring that the provided function is executed without
 * interference from other tasks that might also be trying to access LVGL resources. After the function is executed,
 * the mutex lock is released to allow other tasks to access LVGL functions.
 * @param func A pointer to the function that should be executed while holding the LVGL mutex lock. The function should take no parameters and return void.
 * Example usage:
 * lvgl_execute_locked([]() {
 *    // Your code that interacts with LVGL goes here
 * });
 */
void lvgl_execute_locked(void (*func)(void)) {
    if (s_lvgl_lock(portMAX_DELAY)) {
        func();
        s_lvgl_unlock();
    }
}
