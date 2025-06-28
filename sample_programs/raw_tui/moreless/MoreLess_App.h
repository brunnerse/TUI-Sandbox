#pragma once

#include "TUI_App.h"

#include <stdint.h>
#include <stdio.h>

#include <deque>

class MoreLess_App : public TUI_App {

public:
	MoreLess_App(const char* filename, bool use_alt_screen, bool check_output_tty) :
       TUI_App(use_alt_screen, true, true, true), on_alt_screen(use_alt_screen), check_output_tty(check_output_tty), filename(filename) 
    {}

    ~MoreLess_App();

    int init_graphics();
    int run();

    int repaint_all(); 
    
private:
    const bool on_alt_screen;
    const bool check_output_tty;

    const char *filename;
    FILE* file;  

    uint64_t file_size;
    uint64_t top_byte; // The first byte visible in the top line
    uint64_t bottom_byte; // The byte <after> the last byte in the bottom byte

    std::deque<uint16_t> bytes_per_line;
    
    uint16_t column;

    enum class LineEnd {NO_LF=0, LF_ONLY, CRLF, BOTH};

    LineEnd line_ending;

    void uninit_graphics();

    void draw_status_bar();
    void output_next_lines(uint16_t num_lines);
    void output_prev_line(bool scroll_before = true);

    void determine_CRLF(char prev_lf, char lf);

	virtual void app_handler_window_size_changed();
};

