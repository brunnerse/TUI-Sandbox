#pragma once

#include <stdint.h>

class TUI_App {

public:
	TUI_App(bool use_alt_screen, bool disable_echo, bool disable_canonical) : 
		cfg_use_alt_screen(use_alt_screen), 
		cfg_disable_echo(disable_echo),
		cfg_disable_canonical(disable_canonical)
	{}
//	~TUI_App();

	int start();


protected:
	uint16_t terminal_rows = 0;
	uint16_t terminal_columns = 0;

	volatile bool initialized = false;
	volatile bool running = false;

private:
	// CFG-Settings for terminal init
	const bool cfg_use_alt_screen;
	const bool cfg_disable_echo;
	const bool cfg_disable_canonical;

	int init_terminal();
	int uninit_terminal();

	virtual int init_graphics()=0;
	virtual int run()=0;

	virtual void app_handler_exit();
	virtual void app_handler_window_size_changed(uint16_t new_rows, uint16_t new_columns);


	static void handler_exit(int i);
	static void handler_window_size_changed(int i);


	inline static TUI_App *initialized_instance = nullptr; 
};


