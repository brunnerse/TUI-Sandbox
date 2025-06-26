#include "tc.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include <unistd.h>

#include <string>

#include "ANSI_Escape_Codes.h"



static cursor_pos_t cursor_pos = {0, 0};  


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



cursor_pos_t tc_cursor_get_last_set_pos() {
    return cursor_pos;
}

void tc_cursor_save_pos() {
    printf(ESC_CURSOR_SAVE_POS);
}

void tc_cursor_restore_pos() {
    printf(ESC_CURSOR_RESTORE_POS);
}

void tc_cursor_set_pos(uint16_t row, uint16_t col) {
    printf(ESC_CURSOR_SET_POS, row, col);
    cursor_pos = {row, col};
}


void tc_cursor_set_row(uint16_t row) {
    printf(ESC_CURSOR_SET_ROW, row);
    cursor_pos.row = row;
}

void tc_cursor_set_column(uint16_t col) {
    printf(ESC_CURSOR_SET_COLUMN, col);
    cursor_pos.col = col;
}

void tc_cursor_move_column(int16_t col) {
    printf(((col < 0) ? ESC_CURSOR_MOVE_LEFT : ESC_CURSOR_MOVE_RIGHT),
        abs(col));
    cursor_pos.col = (uint16_t)(cursor_pos.col + col);
}

void tc_cursor_move_row(int16_t row) {
    printf(((row < 0) ? ESC_CURSOR_MOVE_UP : ESC_CURSOR_MOVE_DOWN),
        abs(row));
    cursor_pos.row = (uint16_t)(cursor_pos.row + row);
}

void tc_cursor_move_row_begin(int16_t row) {
    printf(((row < 0) ? ESC_CURSOR_MOVE_UP_TO_LINE_BEGIN : ESC_CURSOR_MOVE_DOWN_TO_LINE_BEGIN),
        abs(row));
    cursor_pos.row = (uint16_t)(cursor_pos.row + row);
    cursor_pos.col = 0;
}


void tc_erase_all() {
    printf(ESC_ERASE_ENTIRE_SCREEN);
}

void tc_erase_line() {
    printf(ESC_ERASE_ENTIRE_LINE);
}

void tc_erase_before_cursor(bool line_only) {
    printf(line_only ? ESC_ERASE_FROM_CURSOR_TO_LINE_BEGIN : ESC_ERASE_FROM_CURSOR_TO_SCREEN_BEGIN);
}

void tc_erase_after_cursor(bool line_only) {
    printf(line_only ? ESC_ERASE_FROM_CURSOR_TO_LINE_END : ESC_ERASE_FROM_CURSOR_TO_SCREEN_END);
}

void tc_erase_characters(uint16_t n_chars)
{
    printf(ESC_ERASE_CHARACTERS, n_chars);
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


// Scroll viewport between top_line and bottom_line <lines> up;  if <lines> is negative, scroll down
void tc_scroll_viewport(int16_t lines, uint16_t viewport_top_row, uint16_t viewport_bottom_row, uint16_t terminal_rows)
{
    if (lines >= 0) 
    {
        printf(
            ESC_SET_VIEWPORT_TOP_BOTTOM_MARGIN ESC_CURSOR_SET_POS ESC_INSERT_LINES ESC_SET_VIEWPORT_TOP_BOTTOM_MARGIN,
            viewport_top_row, viewport_bottom_row, viewport_top_row, 1, lines, 1, terminal_rows); 
    }
    else 
    {
        lines = -lines;
        printf(
            ESC_SET_VIEWPORT_TOP_BOTTOM_MARGIN ESC_CURSOR_SET_POS ESC_REMOVE_LINES ESC_SET_VIEWPORT_TOP_BOTTOM_MARGIN,
            viewport_top_row, viewport_bottom_row, viewport_top_row, 1, lines, 1, terminal_rows);

    }
}

#define TC_CURSOR_REQUEST_HANDLE_UNEXPECTED {return false;}
//#define TC_CURSOR_REQUEST_HANDLE_UNEXPECTED {step = 1;}

bool tc_cursor_request_position(uint16_t *row, uint16_t *col)
{
    // use write and read to overcome any buffer issues with stdin and stdout streams
    write(STDOUT_FILENO, ESC_CURSOR_REQUEST_POS, strlen(ESC_CURSOR_REQUEST_POS));

    // Parse response ESC[#row;#colR
    int iterations = 0;
    char c;
    std::string s;
    int step = 0;
    do {
        ssize_t size = read(STDIN_FILENO, &c, 1);
        if (size > 0)
        {
            switch (step){
                case 0:
                    if (c == ESC)
                        step = 1;
                    else 
                        TC_CURSOR_REQUEST_HANDLE_UNEXPECTED 
                    break;
                case 1:
                    if (c == '[')
                        step = 2;
                    else 
                        TC_CURSOR_REQUEST_HANDLE_UNEXPECTED 
                    break;
                case 2: {
                    if (c == ';') {
                        *row = (uint16_t)atoi(s.c_str());
                        s.clear();
                        step = 3;
                    } else if (!isdigit(c))
                        TC_CURSOR_REQUEST_HANDLE_UNEXPECTED
                    else
                        s.push_back(c);
                    break;
                }
                case 3: {
                    if (c == 'R') {
                        *col = (uint16_t)atoi(s.c_str());
                        return true; // Fully parsed
                    } else if (!isdigit(c))
                        TC_CURSOR_REQUEST_HANDLE_UNEXPECTED
                    else
                        s.push_back(c);
                    break;
                }
                default:
                    assert(0);
            }
        }
    } while (++iterations < 1e6);

    return false;
}

bool tc_test_terminal_size(uint16_t *rows, uint16_t *cols)
{
    // Test cursor position: Set cursor pos to (1000, 1000) so it is at the bottom right corner, than request the cursor position
    uint16_t row_prev, col_prev;
    if (!tc_cursor_request_position(&row_prev, &col_prev))
        return false;
    tc_cursor_set_pos(1000, 1000);
    if (!tc_cursor_request_position(rows, cols))
        return false;
    tc_cursor_set_pos(row_prev, col_prev);
    return true;
}


void tc_write_acs(char acs_c)
{
    printf(ESC_CHARSET_SWITCH_TO_ACS "%c" ESC_CHARSET_SWITCH_TO_ASCII, acs_c);
}

void tc_write_acs(const char *acs_string)
{
    printf(ESC_CHARSET_SWITCH_TO_ACS "%s" ESC_CHARSET_SWITCH_TO_ASCII, acs_string);
}