#include "tc.h"

#include <stdio.h>

#include "ANSI_Escape_Codes.h"


static cursor_pos_t cursor_position;  



void tc_set_mode(Mode mode) {

}

void tc_reset_mode(Mode mode) {

}


void tc_set_color(Color color, bool fg=true) {

}

void tc_set_fg_color_6bit(uint8_t r, uint8_t g, uint8_t b, bool fg=true) {

}

void tc_set_fg_color_rgb(uint8_t r, uint8_t g, uint8_t b, bool fg=true) {

}



void tc_alt_screen_enter() 
{
    puts(ESC_ALT_BUFFER_EN);
    
}

void tc_alt_screen_exit()
{
    puts(ESC_ALT_BUFFER_DIS);
}

void tc_cursor_set_pos(uint16_t x, uint16_t y) {

}


void tc_cursor_move_pos(int16_t x, int16_t y) {

}

void tc_cursor_move_pos_x(int16_t x) {

}

void tc_cursor_move_pos_y(int16_t y) {

}





