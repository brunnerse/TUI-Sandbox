#include "Example_App.h"


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




static uint16_t char_row = 0;

int Example_App::repaint_all() {

    assert(terminal_rows >= 4 && terminal_columns >= 25);

    // Update data of components
    this->comp_win_size->update_winsize_values(terminal_rows, terminal_columns);

    // Update component bounds
    *TUI_App::get_bounds(comp_status.get())     = rectangle_t(terminal_rows-1, 1, 1, terminal_columns);
    *TUI_App::get_bounds(comp_win_size.get())   = rectangle_t(1, terminal_columns-19, 1, 20);
    *TUI_App::get_bounds(comp_time.get())       =  rectangle_t(terminal_rows-5, terminal_columns-24, 3, 25);
    *TUI_App::get_bounds(comp_cmdline.get())    =  rectangle_t(terminal_rows, 1, 1, terminal_columns);
    *TUI_App::get_bounds(comp_text.get())       =  rectangle_t(terminal_rows/2-1, terminal_columns/2-10, 3, 20);

    *TUI_App::get_bounds(comp_exit.get())    =  rectangle_t(terminal_rows/2-2, terminal_columns/2-12, 4, 25);

    // Empty screen and repaint all
    tc_erase_all();
    if (this->status == Status::IDLE) 
    {
        this->comp_win_size->repaint();
        this->comp_status->repaint();
        this->comp_time->repaint();
        this->comp_cmdline->repaint();
        this->comp_text->repaint();

    } 
    else if (this->status == Status::EXIT) 
    {
        this->comp_exit->repaint();
    }

    // Reset character row
    char_row = 0;

    return 0;
}

int Example_App::init_graphics() {

    assert(this->terminal_initialized);

    this->comp_status = std::make_unique<Status_Component>();
    this->comp_win_size = std::make_unique<WindowSize_Component>(terminal_rows, terminal_columns);
    this->comp_time = std::make_unique<Time_Component>(start_time_s);
    this->comp_cmdline = std::make_unique<CommandLine_Component>();
    this->comp_text = std::make_unique<TextBox_Component>("Example App");

    this->comp_exit = std::make_unique<Exit_Component<2>>();
    comp_exit->set_option_text(0, "Yes");
    comp_exit->set_option_text(1, "No");


   this->comp_text->set_cfg(Mode::BLINKING, Color::GREEN, true, Color::BLACK, false);


    this->repaint_all();

    tc_mode_reset();
    tc_cursor_set_pos(terminal_rows, 0);
    
    return 0;
}




void Example_App::app_handler_exit() 
{
    switch(this->status) {
        default:
            this->enter_exit_screen();
            break;
        case Status::EXIT:
            this->mark_for_exit();
            break;
    }
}

int Example_App::run() {
    switch (status) {
        default:
        case Status::IDLE: {
            if (update_time())  {
                this->comp_time->update(current_time_ms, current_time_epoch_s);
                this->comp_cmdline->update();
            }

            int c = getchar();

            if (c != EOF) 
            {
                if (char_row < terminal_rows - 2) {
                    char_row++;
                } else {
                    this->comp_win_size->erase();
                    this->comp_time->erase();
                    this->comp_text->erase();
                    tc_scroll_viewport(-1, 1, terminal_rows-2, terminal_rows);
                    this->comp_win_size->repaint();
                    this->comp_time->repaint();
                    this->comp_text->repaint();
                }

                print_character((char)c, char_row);

                if (c == LF) {
                    std::string command = comp_cmdline->clear();

                    if (process_command(command.c_str())) {
                        status_set((std::string("Processed command: ") + command).c_str());
                    } else {
                        status_set((std::string("Unknown command: ") + command).c_str());
                    }
                    clear_status_time_ms = current_time_ms + 2000;
                } else if (c == DEL) {
                    comp_cmdline->pop_char();
                } else  {
                    comp_cmdline->push_char((char)c);
                }
            }
            if (current_time_ms >= clear_status_time_ms) {// && current_time - clear_status_time < __LONG_MAX__/2) // Catch overflow
                status_set("");
                clear_status_time_ms = __LONG_MAX__;
            }
            break;
        }
        case Status::EXIT: {

            int c = getchar();

            if (c != EOF) {
                if (c == ESC) {
                    std::string escape_expression;
                    do {
                        escape_expression.push_back((char)c);
                        c = getchar();
                    } while (c != EOF && !isspace(c));
                    if (escape_expression.compare(CODE_ESC) == 0) {
                        this->quit_exit_screen();
                    } else if (escape_expression.compare(CODE_ESC"[D") == 0) {
                        comp_exit->select_option(std::max(comp_exit->get_selected_option() - 1u, 0u));
                    } else if (escape_expression.compare(CODE_ESC"[C") == 0) {
                        comp_exit->select_option(std::min(comp_exit->get_selected_option() + 1u, comp_exit->get_num_options() - 1u));
                    }
                } else {
                    if (c == ' ' || c == CODE_LF[0])
                    {
                        if (comp_exit->get_selected_option() == 0)
                            this->mark_for_exit();

                        this->quit_exit_screen();
                    }
                }
            }
            break;
        }
    }


    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    return 0;
}


bool Example_App::process_command(const char* cmd) {
    if (strcmp(cmd, "exit") == 0) {
        this->enter_exit_screen();
        return true;
    } else if (strcmp(cmd, "test") == 0) {
        return true;
    } else if (strcmp(cmd, "clear") == 0) {
        this->repaint_all();
        return true;
    }
    return false;
}


bool Example_App::print_character(char c, uint16_t row) {
    tc_cursor_set_pos(row, 0);
    tc_color_set(Color::CYAN);
    printf("~");
    tc_color_set(Color::WHITE);
    printf("%5u\t\\x%x",  (unsigned)c, (unsigned)c);
    tc_color_set(Color::YELLOW);
    printf("\t'%c'", ('0' <= c && c <= 'z') ? c : '?');
    tc_mode_reset();
    return true;
}

void Example_App::status_set(const char* status) {
    this->comp_status->update_status(status);
}


bool Example_App::update_time() {
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
void Example_App::enter_exit_screen()
{
    this->status = Status::EXIT;
//    tc_save_screen();
    tc_cursor_set_invisible();

    comp_exit->select_option(1);
    this->repaint_all();
}

void Example_App::quit_exit_screen()
{
    this->status = Status::IDLE;
    this->repaint_all();

    tc_cursor_reset_invisible();


}

