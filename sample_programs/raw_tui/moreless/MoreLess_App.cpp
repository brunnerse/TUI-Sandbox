#include "MoreLess_App.h"


#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#include <string>


#include "tc.h"
#include "print_helper.h"

#include "ANSI_Escape_Codes.h"



MoreLess_App::~MoreLess_App()
{
    if (file != nullptr)
        fclose(file);
}

int MoreLess_App::init_graphics() 
{
    assert(this->terminal_initialized);

    file = fopen(filename, "r");
    if (file == nullptr)
    {
        fprintf(stderr, "Could not open file '%s'\n", filename);
    }

    // TODO initialize line and status info, then paint all


    repaint_all();
    
    return 0;
}

int MoreLess_App::repaint_all() 
{
    // TODO print lines of file content and status bar


    
    return 0;
}


// TODO keep file open, load it line by line
// Output info: Give percentage of file on read bytes vs total bytes (like more does)
int MoreLess_App::run() 
{
    // TODO check if blocking (as it is supposed to be)
    int c = getchar();

    if (c != EOF) {
        if (c == LF) {

        } else if (c == 'q') {
            this->mark_for_exit();
        }
    }


    //std::this_thread::sleep_for(std::chrono::milliseconds(1));

    return 0;
}

void MoreLess_App::print_line(const char* line, uint16_t row)
{
    if (this->use_colors)
    {

    }
    else 
    {

    }
}