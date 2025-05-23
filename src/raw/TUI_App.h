#pragma once

#include <stdint.h>

class TUI_App {

public:
	TUI_App() : 
		cfg_use_alt_screen(true), 
		cfg_disable_echo(false),
		cfg_disable_canonical(true)
	{}
//	~TUI_App();

	int start();


protected:
	// CFG-Settings
	const bool cfg_use_alt_screen;
	const bool cfg_disable_echo;
	const bool cfg_disable_canonical;


	uint16_t terminal_rows = 0;
	uint16_t terminal_columns = 0;

	volatile bool initialized = false;
	volatile bool running = false;

private:

	virtual int init_terminal();
	virtual int uninit_terminal();

	virtual int init_graphics()=0;
	virtual int run()=0;

	virtual void app_handler_exit();
	virtual void app_handler_window_size_changed(uint16_t new_rows, uint16_t new_columns);


	static void handler_exit(int i);
	static void handler_window_size_changed(int i);


	inline static TUI_App *initialized_instance = nullptr; 
};


