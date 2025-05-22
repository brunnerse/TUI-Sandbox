#include "TUI_App.h"

#include "terminal_cfg.h"
#include "tc.h"

#include <termios.h>
#include <signal.h>


void TUI_App::handler_exit(int i) 
{
	if (TUI_App::initialized_instance != nullptr)
		TUI_App::initialized_instance->app_handler_exit();

}

void TUI_App::handler_window_size_changed(int i) 
{
	if (TUI_App::initialized_instance != nullptr) {
		TUI_App::initialized_instance->app_handler_window_size_changed();
	}
}


void TUI_App::app_handler_exit()
{
	if (this->initialized) {
		this->uninit();
	}
}

void TUI_App::app_handler_window_size_changed()
{

}



int TUI_App::init()
{
	if (TUI_App::initialized_instance != nullptr) {
		fprintf(stderr, "Only one instance can be active at the same time!");
		return -1;
	}
	TUI_App::initialized_instance = this;

	// Setup terminal config 
	terminal_cfg_store();
	terminal_cfg_set(false, false);
	tc_alt_screen_enter();

	// Setup sigint handler
	signal(SIGINT, handler_exit); 
	signal(SIGWINCH, handler_window_size_changed); 

	tc_cursor_set_pos(0, 0);

	this->initialized = true;
	return 0;
}



int TUI_App::uninit()
{
	tc_alt_screen_exit();
	terminal_cfg_restore();	

	this->initialized = false;
	TUI_App::initialized_instance = nullptr;

	return 0;
}
