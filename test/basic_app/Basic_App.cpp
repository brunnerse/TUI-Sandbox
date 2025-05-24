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

#include "ANSI_Escape_Codes.h"



int Basic_App::repaint_all() {

// TODO Handle when in exit query; e.g. window size is changed while exit query
    assert(terminal_rows > 0 && terminal_columns >= 20);

    // Update data of components
    this->comp_win_size->update_winsize_values(terminal_rows, terminal_columns);

    // Update component bounds
    *TUI_App::get_bounds(comp_status.get())     = rectangle_t(terminal_rows-1, 1, 1, terminal_columns);
    *TUI_App::get_bounds(comp_win_size.get())   = rectangle_t(1, terminal_columns-19, 1, 20);
    *TUI_App::get_bounds(comp_time.get())       =  rectangle_t(terminal_rows-5, terminal_columns-24, 3, 25);
    *TUI_App::get_bounds(comp_cmdline.get())    =  rectangle_t(terminal_rows, 1, 1, terminal_columns);
    *TUI_App::get_bounds(comp_text.get())       =  rectangle_t(terminal_rows/2-1, terminal_columns/2-10, 3, 20);

    // Empty screen and repaint all
    tc_erase_all();
    this->comp_win_size->repaint();
    this->comp_status->repaint();
    this->comp_time->repaint();
    this->comp_cmdline->repaint();
    this->comp_text->repaint();

    return 0;
}

int Basic_App::init_graphics() {

    assert(this->terminal_initialized);

    this->comp_status = std::make_unique<Status_Component>();
    this->comp_win_size = std::make_unique<WindowSize_Component>(terminal_rows, terminal_columns);
    this->comp_time = std::make_unique<Time_Component>(start_time_s);
    this->comp_cmdline = std::make_unique<CommandLine_Component>();
    this->comp_text = std::make_unique<TextBox_Component>("Basic App");

    this->comp_text->set_cfg(Mode::BLINKING, Color::GREEN, true, Color::BLACK, false);


    this->repaint_all();

    tc_mode_reset();
    tc_cursor_set_pos(terminal_rows, 0);
    
    return 0;
}





int Basic_App::run() {

    if (update_time())  {
        this->comp_time->update(current_time_ms, current_time_epoch_s);
        this->comp_cmdline->update();
    }

    int c = getchar();

    if (c != EOF) {

        print_character((char)c);

        if (c == CODE_LF[0]) {
            std::string command = comp_cmdline->clear();

            if (process_command(command.c_str())) {
                status_set((std::string("Processed command: ") + command).c_str());
            } else {
                status_set((std::string("Unknown command: ") + command).c_str());
            }
            clear_status_time_ms = current_time_ms + 2000;
        } else if (c == DEL[0]) {
            comp_cmdline->pop_char();
        } else  {
            comp_cmdline->push_char((char)c);
        }
    }

    if (current_time_ms >= clear_status_time_ms) {// && current_time - clear_status_time < __LONG_MAX__/2) // Catch overflow
        status_set("");
        clear_status_time_ms = __LONG_MAX__;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    return 0;
}


bool Basic_App::process_command(const char* cmd) {
    if (strcmp(cmd, "exit") == 0) {
        // TODO handle exit_screen differently (e.g. state machine) so the normal run() still runs as usual
        this->show_exit_screen();
        return true;
    } else if (strcmp(cmd, "test") == 0) {
        return true;
    }
    return false;
}


// TODO move in extra component
bool Basic_App::print_character(char c) {
    static uint16_t row = 1; //TODO reset row regularly
//    tc_cursor_save_pos();
    tc_cursor_set_pos(row, 0);
    row += 1;

    tc_color_set(Color::CYAN);
    printf("~");
    tc_color_set(Color::WHITE);
    printf("%5u\t\\x%x",  (unsigned)c, (unsigned)c);
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

    this->current_time_epoch_s = time(NULL);
    this->current_time_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (current_time_ms - last_update_time_ms > TIME_UPDATE_INTERVAL_MS){
        last_update_time_ms = current_time_ms;
        return true;
    }
    return false;
}


#define NUM_EXIT_OPTIONS 2

bool Basic_App::show_exit_screen() 
{
//    tc_save_screen();
    tc_cursor_set_invisible();

//    tc_alt_screen_enter();
    tc_erase_all();


    std::unique_ptr<Exit_Component<NUM_EXIT_OPTIONS>> comp_exit = std::make_unique<Exit_Component<NUM_EXIT_OPTIONS>>();

    *TUI_App::get_bounds(comp_exit.get())    =  rectangle_t(terminal_rows/2-2, terminal_columns/2-12, 4, 25);

    comp_exit->set_option_text(0, "Yes");
    comp_exit->set_option_text(1, "No");

    comp_exit->repaint();

    comp_exit->set_option(1);
    

    std::string escape_expression;
    bool escaped = false;

    while (this->running) {
        int c = getchar();

        if (c != EOF) {
            if (c == ESC[0]) {
                escaped = !escaped;
                escape_expression.clear();
            }
            else {
                if (escaped) {
                    escape_expression.push_back((char)c);
                    if (isalpha(c)) {
                        escaped = false;
                        if (escape_expression.compare("[D") == 0) {
                            comp_exit->set_option(std::max(comp_exit->get_option() - 1, 0));
                        } else if (escape_expression.compare("[C") == 0) {
                            comp_exit->set_option(std::min(comp_exit->get_option() + 1, NUM_EXIT_OPTIONS-1));
                        }
                    }
                } else {
                    if (c == ' ' || c == CODE_LF[0])
                        break;
                }
            }
        }
    }

    // Mark for exiting
    if (comp_exit->get_option() == 0) {
        this->running = false;
    } else {
        this->repaint_all();
    }

//    tc_alt_screen_exit();
    tc_cursor_reset_invisible();
//   tc_restore_screen();

    return true;
}