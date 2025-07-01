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

    file = fopen(filename, "rb");
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

    this->bottom_byte = 0;
    this->top_byte = 0;

    this->column = 1;

    //tc_cursor_set_invisible();

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

    tc_print_repeated(' ', (uint16_t)std::max(0, terminal_columns - len1 - len2));

    fwrite(str, (unsigned)len2, 1, stdout);
    tc_mode_reset();
}

void MoreLess_App::app_handler_window_size_changed()
{
    if (on_alt_screen)
    {
        uint16_t prev_rows = terminal_rows;
        this->read_terminal_size();
        // If file has less lines than terminal:  Erase all before for simplicity
        if (this->bytes_per_line.size() < terminal_rows)
            tc_erase_all();
        this->repaint_all();
        if (terminal_rows > prev_rows && top_byte > 0) // If size increased: print line
        {
            for (int row = prev_rows; row < terminal_rows; row++) {
                tc_cursor_set_pos(0,0);
                output_prev_line(false);
            }
        }
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

        if (c == LF || c == ' ') 
        { 
            output_next_lines(1);
        } 
        else if (c == DEL) 
        {
            if (on_alt_screen)
                output_prev_line();
        }
        else if (c == 'q' || c == ESC) 
        {
            this->mark_for_exit();
        }
    }

    return 0;
}

// TODO Handle tabs and non-ascii chars

void MoreLess_App::output_prev_line(bool scroll_before)
{
    // First: read previous terminal_columns bytes
    long pos = std::max(0l, (long)(top_byte - terminal_columns));
    uint16_t n_bytes = (uint16_t)(top_byte - (ulong)pos);
    std::unique_ptr<char> buf = std::make_unique<char>(n_bytes);

    fseek(file, pos, SEEK_SET);
    fread(buf.get(), n_bytes, 1, file);

    // Go reverse in buffer until we get newline; if last char in buffer is newline, skip that
    uint16_t start_line_pos = 0;
    for (long i = n_bytes - 2; i >= 0; i--) {
        if (buf.get()[i] == '\n') {
            start_line_pos = (uint16_t)(i + 1);
            break;
        }
    } 

    uint16_t bytes_to_output = n_bytes - start_line_pos;

    if (bytes_to_output > 0) 
    {
        if (scroll_before)
            tc_scroll_viewport(1, 1, terminal_rows - 1, terminal_rows);
        fwrite(buf.get() + start_line_pos, bytes_to_output, 1, stdout);

        uint16_t prev_end_line_bytes = this->bytes_per_line.back();
        this->bytes_per_line.pop_back();
        this->bytes_per_line.push_front(bytes_to_output);

        bottom_byte -= prev_end_line_bytes;
        top_byte -= bytes_to_output;
    }

    tc_cursor_set_pos(terminal_rows, 1);
    this->draw_status_bar();
}

void MoreLess_App::output_next_lines(uint16_t num_lines)
{
    tc_insert_empty_lines(1);
    fseek(file, (long)bottom_byte, SEEK_SET);

    this->column = 1;

    int prev_c, c = '\n';

    for(uint16_t line = 0; line < num_lines; line++)
    {
        uint16_t bytes_in_line = 0;

        while (true) 
        {
            prev_c = c;
            c = getc(file);

            if (c == EOF)
                break;

            bytes_in_line++;

            if (++this->column > terminal_columns || c == LF) 
            {
                if (c == LF) {
                    determine_CRLF((char)prev_c, (char)c);
                    tc_erase_after_cursor(true); // Erase rest of line
                }

                putchar(LF);
                this->column = 1;
                break;
            } else {
                //if (c == HT)
                //    printf("%*s", 4, ""); 
                // TODO Handle escaped signs differently
                putchar(c);
            }

        }  

        // If printed something: Increase top_byte and bottom_byte, update bytes_per_line
        if (bytes_in_line > 0) {
            this->bottom_byte += bytes_in_line;
            this->bytes_per_line.push_back(bytes_in_line);
            // If erased first line: Adapt top_byte and bytes_per_line
            if (bytes_per_line.size() > terminal_rows - 1u) {
                uint16_t prev_first_line_bytes = this->bytes_per_line.front();
                this->bytes_per_line.pop_front();
                top_byte += prev_first_line_bytes;
            }

        }

        if (c == EOF) {
            tc_erase_after_cursor(true); // Erase rest of line
            // Reached end of file
            if (this->on_alt_screen) {
                // TODO more sophisticated; what if message exceeds last line??
                if (prev_c != '\n') {
                    tc_mode_set(Mode::DIM);
                    printf(" -- No LF at end of file");
                    tc_mode_reset();
                }
            }
            else  {
                this->mark_for_exit();
            }

            break;
        }
    }

    this->draw_status_bar();
}

void MoreLess_App::determine_CRLF(char prev_lf, char lf) 
{
    if (lf != '\n')
        return;
    LineEnd ending = (prev_lf == '\r') ? LineEnd::CRLF : LineEnd::LF_ONLY;

    if (this->line_ending == LineEnd::NO_LF)
        this->line_ending = ending;
    else if (this->line_ending != ending)
        this->line_ending = LineEnd::BOTH;
}