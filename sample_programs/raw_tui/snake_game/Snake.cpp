#include "Snake.h"


#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#include <string.h>

#include <chrono>
#include <thread>
#include <string>

#include "tc.h"

#include "ANSI_Escape_Codes.h"


int Snake::repaint_all() {

    // Update component bounds
    *TUI_App::get_bounds(&comp_exit)  =  rectangle_t(terminal_rows/2-2, terminal_columns/2-12, 4, 25);

    // Empty screen and repaint all
    tc_cursor_set_pos(0,0);
    tc_erase_after_cursor();

    if (this->status == Status::IDLE) 
    {
        this->repaint_game();
    } 
    else if (this->status == Status::EXIT) 
    {
        // TODO dim the game stuff somehow?
        tc_mode_set(Mode::DIM);
        this->repaint_game(); // Paint comp_exit over game
        tc_mode_reset(Mode::DIM);
        this->comp_exit.repaint();
    } else assert(0);

    return 0;
}

int Snake::init_graphics() {

    assert(this->terminal_initialized);

    tc_cursor_set_invisible();

    comp_exit.set_option_text(0, "Yes");
    comp_exit.set_option_text(1, "No");


    this->status = Status::IDLE;


    // Produce an empty screen by
    tc_print_repeated(LF, terminal_rows-1);
    tc_erase_all();
    tc_cursor_set_pos(0,0);
    printf("am here");
    sleep(2); 
    tc_erase_after_cursor();
    sleep(2);

    this->repaint_all();

    tc_mode_reset();
    tc_cursor_set_pos(terminal_rows, 0);
    
    return 0;
}

void Snake::uninit_graphics()
{
    // If not on alt screen: Remove exit 
    if (!on_alt_screen)  {
        this->status = Status::IDLE;
        this->repaint_all();
        tc_cursor_set_pos(terminal_rows, 1);
        putchar(LF); 

    }

    tc_cursor_reset_invisible();
}

void Snake::repaint_game()
{
    tc_cursor_set_pos(0, 0);
//    tc_write_acs(ACS_LARROW ACS_DARROW ACS_RARROW ACS_UARROW"\n\n\r");
    tc_write_acs(ACS_BOARD " " ACS_BLOCK " " ACS_DIAMOND " " ACS_CKBOARD "\r\n");
    tc_write_acs(ACS_ULCORNER ACS_HLINE ACS_HLINE ACS_TTEE ACS_HLINE ACS_URCORNER "\r\n"); 
    tc_write_acs(ACS_LTEE ACS_HLINE ACS_HLINE ACS_PLUS ACS_HLINE ACS_RTEE "\r\n"); 
    tc_write_acs(ACS_VLINE "  " ACS_VLINE " " ACS_VLINE "\r\n"); 
    tc_write_acs(ACS_LLCORNER ACS_HLINE ACS_HLINE ACS_BTEE ACS_HLINE ACS_LRCORNER "\r\n\n"); 
    tc_write_acs(ACS_S1 ACS_S3 ACS_S7 ACS_S9 "r\n\n"); 

}


void Snake::app_handler_exit() 
{
    switch(this->status) {
        default:
            this->enter_exit_screen(); // TODO do this from run() loop to prevent race condition
            break;
        case Status::EXIT:
            this->mark_for_exit();
            break;
    }
}

void Snake::app_handler_window_size_changed()
{
    this->read_terminal_size();
	this->repaint_all();
    // TODO adapt snake, and do this from run() 
}

int Snake::run() 
{
    int c = getchar();

    std::string escape_expression;
    if (c == ESC) {
        // Try to get a full escape expression
        do {
            escape_expression.push_back((char)c);
            c = getchar();
        } while (c != EOF && !isspace(c));
    }


    switch (status) {
        default:
        case Status::IDLE: {

            if (!escape_expression.empty()) 
            {

            }
            else  if (c == 'q' || c == ESC) {
                this->enter_exit_screen();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            break;
        }
        case Status::EXIT: {
            if (escape_expression.compare(CODE_ESC) == 0) { // If received single escape
                this->quit_exit_screen();
            } else if (c == 'a' || escape_expression.compare(CODE_ESC"[D") == 0) {
                    comp_exit.select_option((unsigned)std::max((int)comp_exit.get_selected_option() - 1, 0));
            } else if (c == 'd' || escape_expression.compare(CODE_ESC"[C") == 0) {
                    comp_exit.select_option(std::min(comp_exit.get_selected_option() + 1u, comp_exit.get_num_options() - 1u));
            } else {
                if (c == ' ' || c == CODE_LF[0])
                {
                    if (comp_exit.get_selected_option() == 0)
                        this->mark_for_exit();

                    this->quit_exit_screen();
                }
            }
        }
    }

    return 0;
}


void Snake::enter_exit_screen()
{
    this->status = Status::EXIT;
    //tc_cursor_set_invisible();

    comp_exit.select_option(1);
    this->repaint_all();
}

void Snake::quit_exit_screen()
{
    this->status = Status::IDLE;
    this->repaint_all();

    //tc_cursor_reset_invisible();
}


