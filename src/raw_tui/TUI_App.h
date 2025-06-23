#pragma once

#include <stdint.h>
#include <assert.h>

class TUI_Component;

struct rectangle_t {
	uint16_t row;
	uint16_t col;
	uint16_t height;
	uint16_t width;

	rectangle_t(int row, int col, int height, int width) {
		assert(1 <= row && row <= UINT16_MAX);
		assert(1 <= col && col <= UINT16_MAX);
		assert(0 <= height && height <= UINT16_MAX);
		assert(0 <= width && width <= UINT16_MAX);

		this->row = (uint16_t)row;
		this->col = (uint16_t)col;
		this->height = (uint16_t)height;
		this->width = (uint16_t)width;
	}
};


class TUI_Component {
	friend class TUI_App;
public:
protected:
	rectangle_t bounds = {1,1,0,0};
public:
    virtual bool repaint()=0;
    virtual bool update() {return repaint();}

	virtual void erase(); 
};





class TUI_App {

public:
	TUI_App(bool use_alt_screen, bool disable_echo, bool disable_canonical, bool set_input_nonblocking) : 
		cfg_use_alt_screen(use_alt_screen), 
		cfg_disable_echo(disable_echo),
		cfg_disable_canonical(disable_canonical),
		cfg_set_input_nonblocking(set_input_nonblocking)
	{}
//	~TUI_App();

	int start();


protected:
	uint16_t terminal_rows = 0;
	uint16_t terminal_columns = 0;

	volatile bool terminal_initialized = false;
	volatile bool running = false;

	static rectangle_t* get_bounds(TUI_Component* comp) {return &comp->bounds;} 

	void mark_for_exit() {this->running = false;}

private:
	// CFG-Settings for terminal init
	const bool cfg_use_alt_screen;
	const bool cfg_disable_echo;
	const bool cfg_disable_canonical;
	const bool cfg_set_input_nonblocking;

	int init_terminal();
	int uninit_terminal();

	virtual int init_graphics()=0;
	virtual int run()=0;

	virtual int repaint_all()=0; 



	virtual void app_handler_exit();
	virtual void app_handler_window_size_changed(uint16_t new_rows, uint16_t new_columns);


	static void handler_exit(int i);
	static void handler_window_size_changed(int i);


	inline static TUI_App *initialized_instance = nullptr; 
};



