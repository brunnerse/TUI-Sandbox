#include "Basic_App.h"


#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#include <string.h>

#include <chrono>
#include <thread>
#include <string>


#include "tc.h"
#include "print_helper.h"




int Basic_App::repaint_all() {

    assert(terminal_rows > 0 && terminal_columns >= 20);

    // Update data of components
    this->comp_win_size->update_winsize_values(terminal_rows, terminal_columns);

    // Update component bounds
    *TUI_App::get_bounds(comp_status.get())   = rectangle_t(terminal_rows-1, 1, 1, terminal_columns);
    *TUI_App::get_bounds(comp_win_size.get()) = rectangle_t(1, terminal_columns-19, 1, 20);
    *TUI_App::get_bounds(comp_time.get()) =  rectangle_t(terminal_rows-5, terminal_columns-29, 3, 30);

    // Empty screen and repaint all
    tc_erase_all();
    this->comp_win_size->repaint();
    this->comp_status->repaint();
    this->comp_time->repaint();

    return 0;
}

int Basic_App::init_graphics() {

    assert(this->terminal_initialized);

    this->comp_status = std::make_unique<Status_Component>();
    this->comp_win_size = std::make_unique<WindowSize_Component>(terminal_rows, terminal_columns);
    this->comp_time = std::make_unique<Time_Component>(start_time_s);


    this->repaint_all();

    tc_mode_reset();
    tc_cursor_set_pos(terminal_rows, 0);
    
    return 0;
}





int Basic_App::run() {

    static std::string command;

    if (update_time())  {
        tc_cursor_set_pos(terminal_rows, (uint16_t)command.size());
    }

    tc_cursor_set_pos(2, 0);
    printf("B");
    tc_cursor_set_pos(3, 1);
    printf("C");
    tc_cursor_set_pos(4, 2);
    printf("D");
    tc_cursor_set_pos(5, terminal_columns-1);
    printf("X");
    tc_cursor_set_pos(2, terminal_columns-2);
    printf("Y");
    tc_cursor_set_pos(3, terminal_columns);
    printf("Z");
    tc_cursor_set_pos(4, terminal_columns+1);
    printf("A");

    return 0;

    int c = getchar();

    if (c != EOF) {
        //print_character(c);
        tc_cursor_set_pos(terminal_rows, (uint16_t)command.size());

        if (c == '\n') {
            if (process_command(command.c_str())) {
                status_set((std::string("Processed command: ") + command).c_str());
            } else {
                status_set((std::string("Unknown command: ") + command).c_str());
            }
            clear_status_time_ms = current_time_ms + 2000;

            command.clear();
            tc_erase_line();
        } else if (c == '\x7f') { // DEL
            command.pop_back();
            tc_cursor_move_column(-1);
            tc_erase_after_cursor(true);
        } else  {
            command.push_back((char)c);
            putchar(c);
        }
        tc_cursor_set_pos(terminal_rows, (uint16_t)command.size());
    }

    if (current_time_ms >= clear_status_time_ms) // && current_time - clear_status_time < __LONG_MAX__/2) // Catch overflow
        status_clear();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    return 0;
}


bool Basic_App::process_command(const char* cmd) {
    if (strcmp(cmd, "exit") == 0) {
        // TODO Ask for exit on separate screen
        this->show_exit_screen();
        return true;
    } else if (strcmp(cmd, "test") == 0) {
        return true;
    }
    return false;
}


bool Basic_App::print_character(char c) {
    static uint16_t row = 1;
//    tc_cursor_save_pos();
    tc_cursor_set_pos(row, 0);
    row += 1;

    tc_color_set(Color::CYAN);
    printf("~");
    tc_color_set(Color::WHITE);
    printf("\t%3u\t\\x%x",  (unsigned)c, (unsigned)c);
    tc_color_set(Color::YELLOW);
    printf("\t'%c'", ('0' <= c && c <= 'z') ? c : '?');
//    tc_cursor_restore_pos();
    tc_mode_reset();

    return true;
}

void Basic_App::status_set(const char* status) {
    this->comp_status->update_status(status);
}


bool Basic_App::update_time() {
    static time_t last_update_time_ms = 0;

    this->current_time_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (current_time_ms - last_update_time_ms > TIME_UPDATE_INTERVAL_MS){
        last_update_time_ms = current_time_ms;
        return true;
    }
    return false;
}


bool Basic_App::show_exit_screen() {
// TODO improve
    tc_save_screen();
//    tc_alt_screen_enter();
    tc_erase_all();

    tc_mode_reset();
    
    // Mark for exiting
    if (true) {
        this->running = false;
    }
//    sleep(2);

    tc_erase_all();

    tc_cursor_reset_invisible();
    tc_restore_screen();
//    tc_alt_screen_exit();
    sleep(1);

    return true;
}