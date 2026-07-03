#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*brookesia_diagnostics_status_provider_t)(char* buffer, int buffer_size);

void brookesia_diagnostics_ui_create(brookesia_diagnostics_status_provider_t status_provider);

#ifdef __cplusplus
}
#endif
