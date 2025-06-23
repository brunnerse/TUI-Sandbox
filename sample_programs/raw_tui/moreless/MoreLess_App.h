#pragma once

#include "TUI_App.h"

#include <stdint.h>
#include <stdio.h>



class MoreLess_App : public TUI_App {

public:
	MoreLess_App(const char* filename, bool use_alt_screen, bool use_colors) :
       TUI_App(use_alt_screen, true, true, false), use_colors(use_colors), filename(filename)
    {}

    ~MoreLess_App();

    int init_graphics();
    int run();

    int repaint_all(); 
    
private:
    const bool use_colors;

    const char *filename;
    FILE* file;  

    void print_line(const char* line, uint16_t row);

};

