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


#define TIME_UPDATE_INTERVAL_MS 500

class Text_Component : public TUI_Component {

private:

public:
    Text_Component() 
    {}

    bool update() {

        return this->repaint();
    }

    bool repaint() {

        // TODO move in new component as well
        tc_cursor_set_pos(this->bounds.row, this->bounds.col); 
        tc_mode_set(Mode::BLINKING, Color::GREEN, true, Color::BLACK);
        tc_cursor_save_pos();
        printf("%20s", "");
        tc_cursor_restore_pos();
        tc_cursor_move_row(1);
        printf("%20s", "Basic App      ");
        tc_cursor_restore_pos();
        tc_cursor_move_row(2);
        printf("%20s", "");


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
        printf_aligned(bounds.width, Align::RIGHT, "[ms] %10lu", current_time_ms - start_time_s * 1000);
        tc_cursor_set_pos(bounds.row + 1, bounds.col);
        printf_aligned(bounds.width, Align::RIGHT, "[Epoch] %10lu", current_time_s);
        tc_cursor_set_pos(bounds.row + 2, bounds.col);
        std::string datestr(asctime(localtime(&current_time_s)));
        datestr.pop_back(); // Remove trailing \n
        printf_aligned(bounds.width, Align::RIGHT, "%s", datestr.c_str());

        tc_mode_reset();

        return true;
    }
};

// TODO better way of grouping that?
#define STATUS_MAX_LEN 200

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
