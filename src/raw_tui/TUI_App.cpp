#include "TUI_App.h"

#include "terminal_cfg.h"
#include "tc.h"

#include <termios.h>
#include <signal.h>

void TUI_Component::erase()
{
	tc_cursor_set_pos(bounds.row, bounds.col);
	for (int i = 0; i < bounds.height; i++) { 
		tc_erase_characters(bounds.width);
		tc_cursor_move_row(1);
	}
}

void TUI_App::handler_exit(int i) 
{
	(void)i;
	if (TUI_App::initialized_instance != nullptr)
		TUI_App::initialized_instance->app_handler_exit();

}

void TUI_App::handler_window_size_changed(int i) 
{
	(void)i;
	uint16_t cols, rows;
	terminal_cfg_get_size(&rows, &cols);
	if (TUI_App::initialized_instance != nullptr) {
		TUI_App::initialized_instance->app_handler_window_size_changed(rows, cols);
	}
}


void TUI_App::app_handler_exit()
{
	if (this->running)
		this->running = false;
}

void TUI_App::app_handler_window_size_changed(uint16_t new_rows, uint16_t new_columns)
{
	this->terminal_rows = new_rows;
	this->terminal_columns = new_columns;

	this->repaint_all();
}


int TUI_App::start() 
{
	this->init_terminal();
	this->init_graphics();

	this->running = true;

	while(this->running && 0 == this->run())
		;

	this->uninit_terminal();

	return 0;
}

int TUI_App::init_terminal()
{
	if (TUI_App::initialized_instance != nullptr) {
		fprintf(stderr, "Only one instance can be active at the same time!");
		return -1;
	}
	TUI_App::initialized_instance = this;

	// Setup terminal config 
	terminal_cfg_store();
	terminal_cfg_set(!this->cfg_disable_echo, !this->cfg_disable_canonical, this->cfg_set_input_nonblocking);

	// Do not buffer stdout
	setbuf(stdout, NULL);

	if (this->cfg_use_alt_screen)
		tc_alt_screen_enter();

	// Read terminal size
	if (isatty(STDOUT_FILENO))
	{
		terminal_cfg_get_size(&this->terminal_rows, &this->terminal_columns);
	}
	else
	{
		// Not connected to output terminal; still start the application using default size values
		this->terminal_rows = 100;
		this->terminal_columns = 80;
	}


	// Setup signal handlers
	signal(SIGINT, handler_exit); 
	signal(SIGWINCH, handler_window_size_changed); 

	this->terminal_initialized = true;
	return 0;
}


int TUI_App::uninit_terminal()
{
	if (this->cfg_use_alt_screen)
		tc_alt_screen_exit();

	terminal_cfg_restore();	

	this->terminal_initialized = false;
	TUI_App::initialized_instance = nullptr;

	return 0;
}
