#pragma once

#include "TUI_App.h"
#include "Exit_Component.h"

#include <time.h>
#include <memory>



class Snake : public TUI_App {

public:
	Snake(bool use_alt_screen) :
       		TUI_App(use_alt_screen, true, true, true), on_alt_screen(use_alt_screen)
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

    Exit_Component<2> comp_exit;


};

