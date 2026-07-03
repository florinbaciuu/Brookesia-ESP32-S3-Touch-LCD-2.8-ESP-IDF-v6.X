#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start the Brookesia integration layer.
 *
 * Returns true when the adapter owns the UI entry point. Returns false when the
 * adapter is disabled or not ready, allowing main to start the existing UI.
 */
bool brookesia_adapter_start(void);

#ifdef __cplusplus
}
#endif
