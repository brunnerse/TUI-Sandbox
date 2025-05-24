#pragma once

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#include <string.h>

#include <chrono>
#include <thread>
#include <string>


#include "TUI_App.h"

#include "tc.h"
#include "print_helper.h"


// Constants
#define TIME_UPDATE_INTERVAL_MS 100

#define STATUS_MAX_LEN 200
#define TEXT_MAX_LEN 200



class TextBox_Component : public TUI_Component {

private:
    char text[TEXT_MAX_LEN];

    Mode mode = Mode::NONE;
    Color fg_color = Color::DEFAULT, bg_color = Color::DEFAULT;
    bool fg_bright = false, bg_bright = false;

public:
    TextBox_Component() {text[0] = '\0';}
    TextBox_Component(const char *text) { strncpy(this->text, text, TEXT_MAX_LEN); }

    void set_cfg(Mode mode, Color fg_color, bool fg_bright=false, Color bg_color=Color::DEFAULT, bool bg_bright=false) {
        this->mode = mode;
        this->fg_color = fg_color;
        this->fg_bright = fg_bright;
        this->bg_color = bg_color;
        this->bg_bright = bg_bright;
    }

    bool update(const char* text) {
        strncpy(this->text, text, TEXT_MAX_LEN);
        return this->repaint();
    }


    bool repaint() {
        tc_mode_set(mode, fg_color, fg_bright, bg_color, bg_bright);
        for (uint16_t row = bounds.row; row < bounds.row + bounds.height; row++) {
            tc_cursor_set_pos(row, bounds.col); 
            if ((row - bounds.row) == bounds.height/2) {
                printf_aligned(bounds.width, Align::CENTER, "%s", text);
            } else {
                print_spaces(bounds.width);
            }
        }
        tc_mode_reset();

        return true;
    }
};



class Time_Component : public TUI_Component {

private:
    time_t current_time_ms, current_time_s;
    const time_t start_time_s;

public:
    Time_Component(time_t start_time_s) 
    : start_time_s(start_time_s) 
    {}

    bool update(long current_time_ms, long current_time_s) {
        this->current_time_ms = current_time_ms;
        this->current_time_s = current_time_s;

        return this->repaint();
    }

    bool repaint() {
        assert(bounds.height == 3);
        tc_mode_set(Mode::ITALIC, Color::RED, true, Color::BLACK, true);

        tc_cursor_set_pos(bounds.row, bounds.col);
        printf_aligned(bounds.width, Align::RIGHT, "[ms] %13lu", current_time_ms - start_time_s * 1000);
        tc_cursor_set_pos(bounds.row + 1, bounds.col);
        printf_aligned(bounds.width, Align::RIGHT, "[Epoch] %13lu", current_time_s);
        tc_cursor_set_pos(bounds.row + 2, bounds.col);
        std::string datestr(asctime(localtime(&current_time_s)));
        datestr.pop_back(); // Remove trailing \n
        printf_aligned(bounds.width, Align::RIGHT, "%s", datestr.c_str());

        tc_mode_reset();

        return true;
    }
};


class Status_Component : public TUI_Component {
private:
    char status_text[STATUS_MAX_LEN];

public:
    Status_Component() 
    {
        this->status_text[0] = '\0';
    }


    bool update_status(const char* new_status_text) {
        strncpy(status_text, new_status_text, STATUS_MAX_LEN);
        return this->update();
    }

    bool repaint() {
        return repaint_status(true);
    }

    bool update() {
        return repaint_status(false);
    }

private:
    bool repaint_status(bool repaint_suffix) {
        const char *suffix_text = "[Status]";
        // Draw status line
        tc_cursor_set_pos(bounds.row, bounds.col);
        tc_mode_set(Mode::BOLD, Color::BLACK, Color::WHITE);

        int len_status_text = printf("%s", status_text);
        printf("%*s", (bounds.width - len_status_text - (int)strlen(suffix_text)), "");

        if (repaint_suffix)
            printf("[Status]");

        tc_mode_reset();

        return true;
    }

};

class WindowSize_Component : public TUI_Component {
private:
    uint16_t width;
    uint16_t terminal_width, terminal_height;
public:
    WindowSize_Component(uint16_t term_width, uint16_t term_height) 
    : terminal_width(term_width), terminal_height(term_height)
    {
    }

    void update_winsize_values(uint16_t term_width, uint16_t term_height) {
        this->terminal_width = term_width;
        this->terminal_height = term_height;
    }


    bool repaint() {
        // Print terminal rows and columns in top right
//        char strbuf[100];
//        snprintf(strbuf, 100, "%u rows, %u columns\n", terminal_width, terminal_height);

        tc_cursor_set_pos(bounds.row, bounds.col);
        tc_mode_set(Mode::ITALIC, Color::MAGENTA, true);
        printf_aligned(bounds.width, Align::RIGHT, "%u rows, %u columns", terminal_width, terminal_height);
        tc_mode_reset();

        return true;
    }

};


class CommandLine_Component : public TUI_Component {
private:
    std::string line;
    uint16_t output_width = 0;

public:

    bool push_char(char c) {
        line.push_back(c);
        return update();
    }

    void pop_char() {
        line.pop_back();
        output_width -= 1;
        tc_cursor_set_pos(bounds.row, bounds.col + output_width);
        tc_erase_after_cursor(true);
    }

    std::string clear() {
        std::string cpy = line;
        line.clear();
        output_width = 0;

        repaint();

        return cpy;
    } 

    bool repaint() {
        tc_cursor_set_pos(bounds.row, bounds.col);
        tc_erase_after_cursor(true);

        printf("%s", line.c_str());
        output_width = (uint16_t)line.size();
        return true; 
    }

    bool update() {
        tc_cursor_set_pos(bounds.row, bounds.col + output_width);
        output_width += (uint16_t)printf("%s", &line.c_str()[output_width]);
        return true;
    }
};

class Exit_Component : public TUI_Component {
public:

   bool repaint() {
        tc_cursor_set_pos(bounds.row, bounds.col);
        tc_mode_set(Mode::BOLD, Color::BLACK, false, Color::WHITE, true);
        tc_cursor_save_pos();
        printf("%20s", "");
        tc_cursor_restore_pos();
        tc_cursor_move_row(1);
        tc_cursor_set_invisible();
        printf("%15s%5s", "Exit App?", "");
        tc_cursor_restore_pos();
        tc_cursor_move_row(2);
        printf("%20s", "");

        tc_mode_reset();

        return true;
    }
};
