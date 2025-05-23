#include "tc.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "ANSI_Escape_Codes.h"



static cursor_pos_t cursor_position = {0, 0};  
// TODO Update cursor position in all functions


void tc_mode_set(Mode mode) {
    printf(ESC_MODE, (unsigned)mode);
}

void tc_mode_reset(Mode mode) {
    printf(ESC_MODE, ESC_MODE_RESET_OFFSET + (unsigned)mode);
}

void tc_mode_reset() {
    tc_mode_set(Mode::NONE);
}


void tc_mode_set(Mode mode, Color fg_color, bool fg_bright) {
    printf(ESC_MODE_COLOR, 
        (unsigned)mode,
        (fg_bright ? ESC_MODE_COLOR_FG_BRIGHT_OFFSET : ESC_MODE_COLOR_FG_OFFSET)
        + (unsigned)fg_color);
}

void tc_mode_set(Mode mode, Color fg_color, Color bg_color) {
    printf(ESC_MODE_COLOR_FG_BG, 
        (unsigned)mode,
        ESC_MODE_COLOR_FG_OFFSET + (unsigned)fg_color,
        ESC_MODE_COLOR_BG_OFFSET + (unsigned)bg_color);

}

void tc_mode_set(Mode mode, Color fg_color, bool fg_bright, Color bg_color, bool bg_bright) {
    printf(ESC_MODE_COLOR_FG_BG, 
        (unsigned)mode,
        (fg_bright ? ESC_MODE_COLOR_FG_BRIGHT_OFFSET : ESC_MODE_COLOR_FG_OFFSET)
        + (unsigned)fg_color,
        (bg_bright ? ESC_MODE_COLOR_BG_BRIGHT_OFFSET : ESC_MODE_COLOR_BG_OFFSET)
        + (unsigned)bg_color);
}


void tc_color_set(Color color, bool bright) {
    printf(ESC_MODE, 
        (bright ? ESC_MODE_COLOR_FG_BRIGHT_OFFSET : ESC_MODE_COLOR_FG_OFFSET)
        + (unsigned)color);
}

void tc_color_set_bg(Color color, bool bright) {
    printf(ESC_MODE, 
        (bright ? ESC_MODE_COLOR_BG_BRIGHT_OFFSET : ESC_MODE_COLOR_BG_OFFSET)
        + (unsigned)color);
}

void tc_color_reset(bool fg, bool bg) {
    if (fg)
        tc_color_set(Color::DEFAULT);
    if (bg)
        tc_color_set_bg(Color::DEFAULT);
}


void tc_color_set_rgb(uint8_t r, uint8_t g, uint8_t b, bool bg) {
    printf((bg ? ESC_COLOR_BG_RGB : ESC_COLOR_FG_RGB),
        r, g, b
    );
}


void tc_cursor_save_pos() {
    printf(ESC_CURSOR_SAVE_POS);
}

void tc_cursor_restore_pos() {
    printf(ESC_CURSOR_RESTORE_POS);
}

void tc_cursor_set_pos(uint16_t row, uint16_t col) {
    printf(ESC_CURSOR_SET_POS, row, col);
}


void tc_cursor_set_column(uint16_t col) {
    printf(ESC_CURSOR_SET_COLUMN, col);
}

void tc_cursor_move_column(int16_t col) {
    printf(((col < 0) ? ESC_CURSOR_MOVE_LEFT : ESC_CURSOR_MOVE_RIGHT),
        abs(col));
}

void tc_cursor_move_row(int16_t row) {
    printf(((row < 0) ? ESC_CURSOR_MOVE_UP : ESC_CURSOR_MOVE_DOWN),
        abs(row));
}

void tc_cursor_move_row_begin(int16_t row) {
    printf(((row < 0) ? ESC_CURSOR_MOVE_UP_TO_LINE_BEGIN : ESC_CURSOR_MOVE_DOWN_TO_LINE_BEGIN),
        abs(row));
}



void tc_erase_all() {
    printf(ESC_ERASE_ENTIRE_SCREEN);
}

void tc_erase_line() {
    printf(ESC_ERASE_ENTIRE_LINE);
}

void tc_cursor_set_invisible(){
    printf(ESC_CURSOR_INVISIBLE_EN);
}

void tc_cursor_reset_invisible(){
    printf(ESC_CURSOR_INVISIBLE_DIS);
}

void tc_save_screen() {
    printf(ESC_SCREEN_SAVE);
} 

void tc_restore_screen() {
    printf(ESC_SCREEN_RESTORE);
}


void tc_alt_screen_enter() {
    printf(ESC_ALT_BUFFER_EN);
}

void tc_alt_screen_exit(){
    printf(ESC_ALT_BUFFER_DIS);
}