#pragma once

#include "TUI_App.h"
#include "Exit_Component.h"

#include <time.h>
#include <memory>

#include <deque>



class Snake : public TUI_App {

public:
	Snake(bool use_alt_screen, bool use_borders) :
       		TUI_App(use_alt_screen, true, true, true), on_alt_screen(use_alt_screen), use_borders(use_borders)
	{}

    virtual int init_graphics();
    virtual int run();
    virtual void uninit_graphics();
    
    virtual int repaint_all();

private:

    const bool on_alt_screen;

	virtual void app_handler_exit();
	virtual void app_handler_window_size_changed();

    void enter_exit_screen();
    void quit_exit_screen();

    virtual void repaint_game();

    enum class Status { IDLE, EXIT };

    Status status = Status::IDLE;

    Exit_Component<2> comp_exit = Exit_Component<2>("Exit game?");

    struct pos_t {
        uint16_t x;
        uint16_t y;
    };

    struct dir_t {
        int8_t x;
        int8_t y;
    };

    const bool use_borders;
    std::deque<pos_t> snake;
    pos_t apple;
    dir_t snake_dir;

    bool stopped = true;

    uint32_t current_score, highscore;


    void game_init(); 
    void game_update(); 
    void game_stop(pos_t collision_pos); 

    void caught_apple();
    void update_score(); 

    void update_dir(int8_t x, int8_t y); 
    pos_t get_rand_pos(); 
    cursor_pos_t pos_to_cursor(pos_t pos);
    void draw_apple();
    void draw_snake();
    bool is_pos_in_snake(pos_t pos);



};

