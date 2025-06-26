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
//        tc_scroll_viewport(-10, 1, terminal_rows, terminal_rows);
    // TODO print lines of file content and status bar
    printf("\r\n\n");

    tc_write_acs(CODE_SI ACS_LARROW ACS_DARROW ACS_RARROW ACS_UARROW CODE_SO "\n\n\r");
    tc_write_acs(ACS_BOARD " " ACS_BLOCK " " ACS_DIAMOND " " ACS_CKBOARD "\r\n");
    tc_write_acs(ACS_ULCORNER ACS_HLINE ACS_HLINE ACS_TTEE ACS_HLINE ACS_URCORNER "\r\n"); 
    tc_write_acs(ACS_LTEE ACS_HLINE ACS_HLINE ACS_PLUS ACS_HLINE ACS_RTEE "\r\n"); 
    tc_write_acs(ACS_VLINE "  " ACS_VLINE " " ACS_VLINE "\r\n"); 
    tc_write_acs(ACS_LLCORNER ACS_HLINE ACS_HLINE ACS_BTEE ACS_HLINE ACS_LRCORNER "\r\n\n"); 
    tc_write_acs(ACS_S1 ACS_S3 ACS_S7 ACS_S9 "r\n\n"); 

    return 0;
}


// TODO keep file open, load it line by line
// Output info: Give percentage of file on read bytes vs total bytes (like more does)
int MoreLess_App::run() 
{
    // TODO check if blocking (as it is supposed to be)
    int c = getc(stdin);

    fprintf(stderr, "Got %c", c);

    if (c != EOF) {
        if (c == LF) {

        } else if (c == 'q') {
            this->mark_for_exit();
        }
    }
    else
        fprintf(stderr, "Got EOF");



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