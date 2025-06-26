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
	if (TUI_App::initialized_instance != nullptr) {
		TUI_App::initialized_instance->app_handler_window_size_changed();
	}
}


void TUI_App::app_handler_exit()
{
	this->mark_for_exit();
	// If input is set blocking: Send cursor position request so the terminal responds and getchar() unblocks
	if (!cfg_set_input_nonblocking)
    	write(STDOUT_FILENO, ESC_CURSOR_REQUEST_POS, sizeof(ESC_CURSOR_REQUEST_POS));
}

void TUI_App::app_handler_window_size_changed()
{
	this->read_terminal_size();
	this->repaint_all();
}


void TUI_App::read_terminal_size() 
{
	// First try reading the terminal cfg,
	// if that does not work (e.g. not connected to terminal) try using it via escape expressions (tc_test_terminal_size)
	uint16_t rows, cols;
	if (this->is_connected_to_terminal && terminal_cfg_get_size(&rows, &cols) == 0)
	{
	} else if (tc_test_terminal_size(&rows, &cols))
	{ 
	} else
	{
		// Assign default values
		rows = 20;
		cols = 80;
	}


	this->terminal_rows = rows;
	this->terminal_columns = cols;
}


int TUI_App::start() 
{
	bool is_output_a_terminal = isatty(STDOUT_FILENO);
	bool is_input_a_terminal = isatty(STDIN_FILENO);

	if (cfg_use_alt_screen) {
		// TODO display this warning also when not using alt screen?
		if (!is_input_a_terminal)
			fprintf(stderr, "Warning: Input is not a terminal\n");
		if (!is_output_a_terminal)
			fprintf(stderr, "Warning: Output is not a terminal\n");
		if (!is_output_a_terminal || !is_input_a_terminal)
			sleep(1);
	}

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
	this->is_connected_to_terminal = terminal_is_connected();
	if (this->is_connected_to_terminal)
	{
		terminal_cfg_store();
		terminal_cfg_set(!this->cfg_disable_echo, !this->cfg_disable_canonical, this->cfg_set_input_nonblocking);
	}

	// Do not buffer stdout
	setbuf(stdout, NULL);

	if (this->cfg_use_alt_screen)
		tc_alt_screen_enter();


	this->read_terminal_size();

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
