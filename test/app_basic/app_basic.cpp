#include "app_basic.h"


#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>


#include "tc.h"

void Basic_App::app_handler_window_size_changed(uint16_t new_rows, uint16_t new_columns) {
    this->terminal_rows = new_rows;
    this->terminal_columns = new_columns;
    this->init_graphics();
}

int Basic_App::init_graphics() {

    assert(this->initialized);

    start_time = time(NULL);

    // Empty screen
    tc_erase_all();
    tc_cursor_set_pos(0,0);

    // Print terminal rows and columns in top right
    char strbuf[100];
    int strbuf_len = snprintf(strbuf, 100, "%u rows, %u columns\n", terminal_rows, terminal_columns);

    tc_cursor_set_pos(0, terminal_columns - strbuf_len);
    tc_mode_set(Mode::ITALIC, Color::MAGENTA, true);
    printf("%s", strbuf);
    tc_mode_reset();


    // Draw status line
    tc_cursor_set_pos(terminal_rows-1, 0);
    tc_mode_set(Mode::BOLD, Color::BLACK, Color::WHITE);
    printf("%*s", terminal_columns, " ");

    tc_cursor_move_column(-(int16_t)sizeof("[Status]")); 
    printf("[Status]");

    tc_mode_reset();


    tc_cursor_set_pos(terminal_rows/2-1, terminal_columns/2-10);
    tc_mode_set(Mode::BLINKING);
    tc_color_set(Color::GREEN, true);
    tc_color_set_bg(Color::BLACK);

    tc_cursor_save_pos();
    printf("%20s", "");
    tc_cursor_restore_pos();
    tc_cursor_move_row(1);
    printf("%20s", "Basic App      ");
    tc_cursor_restore_pos();
    tc_cursor_move_row(2);
    printf("%20s", "");

    tc_mode_reset();
    tc_cursor_set_pos(terminal_rows, 0);
    
    return 0;
}

int Basic_App::run() {

    //char c = getchar();

/*   if (time(NULL) - start_time > 2)
        return -1;
*/

    return 0;
}


