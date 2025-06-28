#include "MoreLess_App.h"


#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>

#include <string>
#include <filesystem>
#include <memory>

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
        return -1;
    }

    if (this->check_output_tty) {
        if (!isatty(STDOUT_FILENO))
        {
            int fd = fileno(file);

            auto buffer = std::make_unique<char>(PIPE_BUF);
            ssize_t size;
            while ((size = read(fd, buffer.get(), PIPE_BUF)) > 0)
                write(STDOUT_FILENO, buffer.get(), (size_t)size);

            return 1;
        }
    }

    std::filesystem::path file_path = filename; 
    this->file_size = std::filesystem::file_size(file_path);

    this->CRLF_lines = 0;
    this->bottom_byte = 0;
    this->top_byte = 0;
    this->end_reached = false;

    this->column = 1;

    tc_cursor_set_invisible();

    repaint_all();
    
    return 0;
}

void MoreLess_App::uninit_graphics() 
{
    if (file != nullptr) {
        fclose(file);
        file = nullptr;
    }

    // Remove status bar if it was drawn (not necessary for alt screen)
    if (!on_alt_screen)  {
        tc_remove_lines(1);
        if (this->column != 1) {
            tc_cursor_move_row(-1);
            tc_cursor_set_column(this->column);
        }
    }

    tc_erase_after_cursor(true);
    tc_cursor_reset_invisible();
}

void MoreLess_App::draw_status_bar()
{
    static const char* LineEnd_Str[4] = {"No LF", "LF", "CRLF", "CRLF & LF"};

    // If in middle of line: Go to begin of next line
    if (this->column != 1) 
        putchar(LF);

    tc_mode_set(Mode::BOLD); 
    tc_mode_set(Mode::INVERSE); 

    int len1;
//    if (on_alt_screen)
//        len1 = printf("More (%lu %% - %lu%%)", 100 * top_byte / file_size, 100 * bottom_byte / file_size);
//    else
        len1 = printf("More (%lu%%)", 100 * bottom_byte / file_size);
    char str[30];
    int len2 = snprintf(str, 30, "%.1f / %.1f KB   %s ", (float)bottom_byte / 1e3f, (float)file_size / 1e3f, 
        LineEnd_Str[(unsigned)line_ending]);
    assert(len1 >= 0 && len2 >= 0);

    tc_print_repeated(' ', (uint32_t)std::max(0, terminal_columns - len1 - len2));

    fwrite(str, (unsigned)len2, 1, stdout);
    tc_mode_reset();
}

void MoreLess_App::app_handler_window_size_changed()
{
    if (on_alt_screen)
    {
        this->read_terminal_size();
        this->repaint_all();
    } else {
        uint16_t prev_columns = terminal_columns;
        this->read_terminal_size();
        if (prev_columns > terminal_columns)  // Handle shrinking:
        {
            tc_cursor_move_row(-1);
            tc_remove_lines(2);
        } 
//            tc_cursor_set_pos(terminal_rows, 1);
        if (prev_columns != terminal_columns) {
            tc_cursor_set_column(1);
            this->draw_status_bar();
        }
    }
}

int MoreLess_App::repaint_all() 
{
    fseek(file, (int64_t)top_byte, 0);

    bottom_byte = top_byte;
    this->bytes_per_line.clear();

    this->output_next_lines((uint16_t)(terminal_rows - 1u));

/*
    printf("\r\n\n");

//    tc_write_acs(ACS_LARROW ACS_DARROW ACS_RARROW ACS_UARROW"\n\n\r");
    tc_write_acs(ACS_BOARD " " ACS_BLOCK " " ACS_DIAMOND " " ACS_CKBOARD "\r\n");
    tc_write_acs(ACS_ULCORNER ACS_HLINE ACS_HLINE ACS_TTEE ACS_HLINE ACS_URCORNER "\r\n"); 
    tc_write_acs(ACS_LTEE ACS_HLINE ACS_HLINE ACS_PLUS ACS_HLINE ACS_RTEE "\r\n"); 
    tc_write_acs(ACS_VLINE "  " ACS_VLINE " " ACS_VLINE "\r\n"); 
    tc_write_acs(ACS_LLCORNER ACS_HLINE ACS_HLINE ACS_BTEE ACS_HLINE ACS_LRCORNER "\r\n\n"); 
    tc_write_acs(ACS_S1 ACS_S3 ACS_S7 ACS_S9 "r\n\n"); 
*/

    return 0;
}


// Output info: Give percentage of file on read bytes vs total bytes (like more does)
int MoreLess_App::run() 
{
    int c = getc(stdin);

    if (c != EOF) {

        if (c == ESC) {
            std::string expr;
            c = getc(stdin);
            while (c != EOF) {
                expr.push_back((char)c);
                if (isalpha(c))
                    break;
                c = getc(stdin);
            }
            // Map to another character depending on what the expression is supposed to do
            if (expr.empty())
                c = ESC;
            else if (expr.compare("[A") == 0)
                c = DEL;
            else if (expr.compare("[B") == 0)
                c = LF;
        }

        if (c == LF || c == ' ') { 
            print_next_line();

            if (this->end_reached && !this->on_alt_screen)
                this->mark_for_exit();
            else
                draw_status_bar();
        } else if (c == DEL) {
            if (on_alt_screen) {
                // TODO scroll back in file too somehow, print previous line of file  
                // if not at beginning of file:
                tc_scroll_viewport(1, 1, terminal_rows - 1, terminal_rows);
            }
        } else if (c == 'q' || c == ESC) {
            this->mark_for_exit();
        }
    }

    return 0;
}

void MoreLess_App::print_next_line()
{
    tc_cursor_set_row(terminal_rows);
    tc_insert_empty_lines(1);

    this->column = 1;

    int c = getc(file);

    while (c != EOF) 
    {
        bottom_byte++;

        if (++this->column > terminal_columns || c == '\n') {
            putchar('\n');
            this->column = 1;
            break;
        } else {
            putchar(c);
        }

        c = getc(file);
    } 
    this->end_reached = (c == EOF);
}