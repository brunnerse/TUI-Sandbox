#include "app_basic.h"


#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>


#include "tc.h"

void Basic_App::app_handler_window_size_changed(uint16_t new_rows, uint16_t new_columns) {

}

int Basic_App::init_graphics() {

    assert(this->initialized);

    start_time = time(NULL);

    // Empty screen
    tc_erase_all();
    tc_cursor_set_pos(0,0);
    printf("I have %u rows, %u columns\n", terminal_rows, terminal_columns);
    // Draw status line
    tc_cursor_set_pos(terminal_rows-2, 0);

    tc_color_set_bg(Color::WHITE);
    tc_color_set(Color::BLACK);

    for (unsigned i = 0; i < terminal_columns; i++)
        printf(" ");
    tc_cursor_move_column(-10); 

    tc_mode_set(Mode::BOLD);
    printf("[Status]");
    tc_mode_reset_all();

    tc_cursor_set_pos(terminal_rows/2, terminal_columns/2);
    tc_mode_set(Mode::ITALIC);
    tc_color_set_bg(Color::GREEN, true);
    tc_color_set(Color::BLACK);

    tc_cursor_save_pos();
    printf("%20s", "");
    tc_cursor_restore_pos();
    tc_cursor_move_row(1);
    printf("%20s", "  Basic App");
    tc_cursor_restore_pos();
    tc_cursor_move_row(2);
    printf("%20s", "");

    tc_mode_reset_all();
    tc_cursor_set_pos(terminal_rows-1, 0);
    
    return 0;
}

int Basic_App::run() {

    //char c = getchar();

/*   if (time(NULL) - start_time > 2)
        return -1;
*/

    return 0;
}


