#pragma once

#include <stdint.h>

int terminal_cfg_store();
int terminal_cfg_restore();


void terminal_cfg_set(bool echo, bool canonical);

int terminal_cfg_get_size(uint16_t *cols, uint16_t *rows);

