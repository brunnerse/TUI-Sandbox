#include "Snake.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h> 
#include <signal.h>

#include <string.h>

#include <chrono>
#include <thread>
#include <string>

#include "tc.h"

#include "ANSI_Escape_Codes.h"


int Snake::init_graphics() {

    assert(this->terminal_initialized);

    tc_cursor_set_invisible();

    comp_exit.set_option_text(0, "Yes");
    comp_exit.set_option_text(1, "No");


    this->status = Status::IDLE;

    srandom((unsigned)time(NULL));
    this->highscore = 0;
    this->game_init();

    // Produce an empty screen by
    tc_print_repeated(LF, terminal_rows-1);
    tc_erase_all();

    this->repaint_all();

    tc_mode_reset();
    tc_cursor_set_pos(terminal_rows, 0);
    
    return 0;
}



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

void Snake::uninit_graphics()
{
    // If not on alt screen: Remove exit 
    if (!on_alt_screen)  {
        this->status = Status::IDLE;
        this->repaint_all();
        tc_cursor_set_pos(terminal_rows, 1);
        putchar(LF); 

    }

    /*
    for (unsigned i = 0; i < 256; i++) 
    {
        printf("\n%u (%x) '%c': \t",i, i, i);
        tc_write_acs(i);
    }
    */ 

    tc_cursor_reset_invisible();
}



void Snake::repaint_game()
{
    tc_cursor_set_pos(0, 0);
    if (use_borders)
    {
        tc_write_acs(ACS_ULCORNER);
        tc_write_acs_repeated(ACS_HLINE, terminal_columns - 2);
        tc_write_acs(ACS_URCORNER);
        tc_cursor_set_pos(terminal_rows, 1);
        tc_write_acs(ACS_LLCORNER);
        tc_write_acs_repeated(ACS_HLINE, terminal_columns - 2);
        tc_write_acs(ACS_LRCORNER);
        for (uint16_t l = 0; l < 2u; l++) {
            tc_cursor_set_pos(2, terminal_columns * l);
            for (uint16_t i = 0; i < terminal_rows - 2u; i++) {
                tc_write_acs(ACS_VLINE);
                if (l == 0)
                    tc_cursor_move_row_begin(1);
                else
                    tc_cursor_move_row(1);
            }
        }
    }

    draw_snake();
    draw_apple();
    update_score();
}



void Snake::draw_apple()
{
    tc_cursor_set_pos(pos_to_cursor(apple));
    tc_mode_set(Mode::BLINKING);
    tc_color_set(Color::RED);
    tc_write_acs(ACS_DIAMOND);
    tc_mode_reset(Mode::BLINKING);
    tc_color_set(Color::DEFAULT);
}

void Snake::draw_snake()
{
    assert(snake.size() > 0);
    tc_color_set(Color::GREEN, true); 
    for (auto iter_pos = snake.rbegin() + 1; iter_pos != snake.rend(); iter_pos++)
    {
        tc_cursor_set_pos(pos_to_cursor(*iter_pos));
        tc_write_acs(ACS_CKBOARD ACS_CKBOARD);
    }

    // Draw head of snake
    tc_cursor_set_pos(pos_to_cursor(snake.back()));
    if (this->stopped) {
        tc_color_set(Color::RED, true);
    } else {
        tc_color_set(Color::GREEN, false); 
    }
    tc_write_acs(ACS_CKBOARD ACS_CKBOARD);

    tc_color_set(Color::DEFAULT, false, Color::DEFAULT, false);
}

bool Snake::is_pos_in_snake(pos_t p)
{
    for (pos_t pos : snake)
    {
        if (p.x == pos.x && p.y == pos.y)
            return true;
    }
    return false;
}

void Snake::caught_apple()
{
    current_score += 1;
    if (current_score > highscore)
        highscore = current_score;

    update_score();
    // No need to erase old apple since overdrawn by snake

    do  {
        apple = get_rand_pos();
    } while (is_pos_in_snake(apple));

    draw_apple();
    putchar(BELL);
    putchar(BELL);
}

Snake::pos_t Snake::get_rand_pos() 
{
    pos_t pos; 

    if (use_borders) {
        pos.x = (uint16_t) ( random() % ((terminal_columns-2)/2) );
        pos.y = (uint16_t) ( random() % (terminal_rows-2) );
//        pos.x = (uint16_t) ( random() % ((terminal_columns-2)/2) );
//        pos.y = (uint16_t) ( random() % (terminal_rows-2) );
    }
    else 
    {
        pos.x = (uint16_t) ( random() % (terminal_columns/2) );
        pos.y = (uint16_t) ( random() % terminal_rows );
    }
    
    return pos;
}

cursor_pos_t Snake::pos_to_cursor(pos_t pos) 
{
    if (use_borders)
        return (cursor_pos_t){(uint16_t)(pos.y + 2u), (uint16_t)(pos.x * 2u + 2u)};
//        return (cursor_pos_t){(uint16_t)(pos.y/2u + 2u), (uint16_t)(pos.x + 2u)};
    else
        return (cursor_pos_t) {(uint16_t)(pos.y + 1u), (uint16_t)(pos.x * 2u + 1u)};
//        return (cursor_pos_t) {(uint16_t)(pos.y/2u + 1u), (uint16_t)(pos.x + 1u)};
}


void Snake::update_score() 
{
    // TODO draw and redraw score + highscore

}

void Snake::game_init() 
{
    this->current_score = 1; // Score equals length of snake
    this->stopped = false;

//    snake 
    snake.clear();
    snake.push_back(get_rand_pos());

    apple = get_rand_pos();

    snake_dir.x = 0; snake_dir.y = 0;
}

void Snake::game_update()
{
    if (snake_dir.x == 0 && snake_dir.y == 0)
        return;
    
    pos_t next_head = snake.back();
    next_head.x += snake_dir.x;
    next_head.y += snake_dir.y;

    if (use_borders) {
        if (next_head.x >= terminal_columns/2 - 1u || next_head.y >= terminal_rows - 2u) {
            printf("%u %u\n", terminal_columns, terminal_rows);
            printf("%u %u\n", next_head.x, next_head.y);
            game_stop(next_head);
            return;
       }
    }
    else {
        // Make snake reappear on other side
        if (next_head.x == UINT16_MAX)
            next_head.x = terminal_columns/2 - 1u;
        if (next_head.x == terminal_columns/2)
            next_head.x = 0;
        if (next_head.y == UINT16_MAX)
            next_head.y = terminal_rows - 1u;
        if (next_head.y == terminal_rows)
            next_head.y = 0;
    }

    // Draw current head light
    tc_color_set(Color::GREEN, true);
    tc_cursor_set_pos(pos_to_cursor(snake.back()));
    tc_write_acs(ACS_CKBOARD ACS_CKBOARD);

    if (is_pos_in_snake(next_head)) 
    {
        // Draw next head in read
        game_stop(next_head);
        return;
    }


    snake.push_back(next_head);

    // draw next head
    tc_color_set(Color::GREEN, false);
    tc_cursor_set_pos(pos_to_cursor(snake.back()));
    tc_write_acs(ACS_CKBOARD ACS_CKBOARD);
    tc_color_set(Color::DEFAULT);

    if (next_head.x == apple.x && next_head.y == apple.y)
    {
        caught_apple();
    }
    else {
        // Erase end of snake
        tc_cursor_set_pos(pos_to_cursor(snake.front()));
        tc_erase_characters(2);
        snake.pop_front();
    } 
}

void Snake::game_stop(pos_t collision_pos) 
{
    this->stopped = true;

    putchar(BELL);
    putchar(BELL);

    snake.push_back(collision_pos);
    draw_snake();
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

    // Adapt snake and redraw apple position 
    for (auto iter = snake.begin(); iter != snake.end(); iter++) { 
        if ( iter->x >= terminal_columns/2 )
            iter->x = (iter->x - terminal_columns/2); 
        if (iter->y >= terminal_rows )
            iter->y = (iter->y - terminal_rows); 
    }
    while (apple.x >= terminal_columns/2 || apple.y >= terminal_rows || is_pos_in_snake(apple)) 
    {
        apple = get_rand_pos();
    } 

	this->repaint_all();
    // TODO do this from run()  to prevent race condition
}

void Snake::update_dir(int8_t x, int8_t y)
{
    if (x != 0 && snake_dir.x == 0) {
        snake_dir.y = 0;
        snake_dir.x = x;
    }
    else if (y != 0 && snake_dir.y == 0) {
        snake_dir.x = 0;
        snake_dir.y = y;
    }
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

            if (!escape_expression.empty() || c != EOF) 
            {
                if (!stopped) {
                    if (c == 'a' || escape_expression.compare(ESC_ARROW_LEFT) == 0) 
                        update_dir(-1, 0);
                    else if (c == 'd' || escape_expression.compare(ESC_ARROW_RIGHT) == 0) 
                        update_dir(1, 0);
                    else if (c == 'w' || escape_expression.compare(ESC_ARROW_UP) == 0)
                        update_dir(0, -1);
                    else if (c == 's' || escape_expression.compare(ESC_ARROW_DOWN) == 0)
                        update_dir(0, 1);
                }
            }

            if  (c == 'q' || escape_expression.compare(CODE_ESC) == 0) {
                this->enter_exit_screen();
            } else if (c == ' ' || c == LF) {
                if (stopped || (snake_dir.x == 0 && snake_dir.y == 0)) {
                    game_init();
                    repaint_all();
                }
            }

            if (!stopped)
                game_update();

            // TODO wait for remainder of time, not total time
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
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


