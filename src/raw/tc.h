#pragma once

#include <stdio.h>
#include <stdint.h>


struct cursor_pos_t {
	uint16_t x;
	uint16_t y;
};


enum class Mode {
	NONE = 0
};


enum class Color {
	WHITE=8
};


void tc_alt_screen_enter();
void tc_alt_screen_exit();

void tc_cursor_set_pos(uint16_t x, uint16_t y);

inline void tc_cursor_set_pos(cursor_pos_t pos) {
	tc_cursor_set_pos(pos.x, pos.y);
}

void tc_cursor_move_pos(int16_t x, int16_t y);
void tc_cursor_move_pos_x(int16_t x);
void tc_cursor_move_pos_y(int16_t y);


