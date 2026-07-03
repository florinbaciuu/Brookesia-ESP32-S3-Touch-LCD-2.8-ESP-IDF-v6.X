#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool brookesia_project_board_init(void);
void brookesia_project_board_deinit(void);
void brookesia_project_board_get_status(char* buffer, int buffer_size);
void brookesia_storage_automount_start(void);

#ifdef __cplusplus
}
#endif
