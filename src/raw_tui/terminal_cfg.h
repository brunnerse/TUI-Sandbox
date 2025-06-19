#pragma once

#include <stdint.h>

int terminal_cfg_store();
int terminal_cfg_restore();


void terminal_cfg_set(bool echo, bool canonical, bool input_nonblocking);

int terminal_cfg_get_size(uint16_t *rows, uint16_t *cols);

