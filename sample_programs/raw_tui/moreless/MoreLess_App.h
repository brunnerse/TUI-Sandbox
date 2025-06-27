#pragma once

#include "TUI_App.h"

#include <stdint.h>
#include <stdio.h>



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
    uint64_t top_byte;
    uint64_t bottom_byte;
    
    uint16_t column;

    bool end_reached;

    uint32_t CRLF_lines;

    const char *line_ending = "LF";

    void uninit_graphics();

    void draw_status_bar();
    void print_next_line();

};

