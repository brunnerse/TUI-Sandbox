#pragma once

class TUI_App {

public:
//	TUI_App();
//	~TUI_App();

	virtual int init();
	virtual int uninit();

	virtual int run()=0;

// TODO make these private later
	virtual void app_handler_exit();
	virtual void app_handler_window_size_changed();

private:
	static void handler_exit(int i);
	static void handler_window_size_changed(int i);

	inline static TUI_App *initialized_instance = nullptr; 

	bool initialized = false;
};


