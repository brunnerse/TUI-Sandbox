#pragma once

#include <stdio.h>
#include <stdint.h>

#include "ANSI_Escape_Codes.h"


struct cursor_pos_t {
	uint16_t row;
	uint16_t col;
};


void tc_mode_set(Mode mode);
void tc_mode_reset(Mode mode);
void tc_mode_reset();

void tc_mode_set(Mode mode, Color fg_color, bool fg_bright=false);
void tc_mode_set(Mode mode, Color fg_color, Color bg_color);
void tc_mode_set(Mode mode, Color fg_color, bool fg_bright, Color bg_color, bool bg_bright=false);


void tc_color_set(Color color, bool bright=false);
void tc_color_set_bg(Color color, bool bright=false);
void tc_color_reset(bool fg=true, bool bg=false);

void tc_color_set_265(uint8_t r, uint8_t g, uint8_t b, bool bg=true); // Not implemented
void tc_color_set_rgb(uint8_t r, uint8_t g, uint8_t b, bool bg=true);



cursor_pos_t tc_cursor_get_last_set_pos();

void tc_cursor_save_pos();
void tc_cursor_restore_pos();

void tc_cursor_set_pos(uint16_t row, uint16_t col);
void tc_cursor_set_column(uint16_t col);

inline void tc_cursor_set_pos(cursor_pos_t pos) {
	tc_cursor_set_pos(pos.row, pos.col);
}

void tc_cursor_move_column(int16_t col);
void tc_cursor_move_row(int16_t row);
void tc_cursor_move_row_begin(int16_t row);


void tc_erase_all();
void tc_erase_line();
void tc_erase_before_cursor(bool line_only=false);
void tc_erase_after_cursor(bool line_only=false);


void tc_cursor_set_invisible();
void tc_cursor_reset_invisible();

void tc_save_screen();
void tc_restore_screen();

void tc_alt_screen_enter();
void tc_alt_screen_exit();