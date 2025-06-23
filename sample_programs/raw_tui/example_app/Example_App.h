#pragma once

#include "TUI_App.h"
#include "Example_App_Components.h"

#include <time.h>

#include <memory>



class Basic_App : public TUI_App {

public:
	Basic_App() :
       TUI_App(true, true, true, true),
       start_time_s(time(NULL))
	{
    }

    virtual int init_graphics();
    virtual int run();
    
    virtual int repaint_all();

private:

    enum class Status {
        IDLE, 
        EXIT
    };

    Status status = Status::IDLE;

    const time_t start_time_s;
    time_t current_time_ms;
    time_t current_time_epoch_s;

    time_t clear_status_time_ms = __LONG_MAX__;

    bool process_command(const char* cmd);

    void status_set(const char* status);
    void status_clear();

    bool update_time();
    bool show_exit_screen();
    bool print_character(char c, uint16_t row);

    std::unique_ptr<Status_Component> comp_status = nullptr;
    std::unique_ptr<WindowSize_Component> comp_win_size = nullptr;
    std::unique_ptr<Time_Component> comp_time = nullptr;
    std::unique_ptr<TextBox_Component> comp_text = nullptr;
    std::unique_ptr<CommandLine_Component> comp_cmdline = nullptr;
    std::unique_ptr<Exit_Component<2>> comp_exit = nullptr;
};

