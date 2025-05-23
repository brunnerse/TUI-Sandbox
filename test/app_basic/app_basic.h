#include "TUI_App.h"

#include <time.h>

class Basic_App : public TUI_App {

public:
	Basic_App() : TUI_App(true, true, true) 
	{}

	virtual void app_handler_window_size_changed(uint16_t new_rows, uint16_t new_columns);

    virtual int init_graphics();
    virtual int run();

private:
    time_t start_time;
};

