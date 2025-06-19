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
#define EXIT_OPTION_MAX_LEN 15



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


class Player_Component : public TUI_Component {

private:
    uint16_t width, height;

    uint16_t pos_x, pos_y;

public:
    Player_Component() {}


    bool update(const char* text) {
        strncpy(this->text, text, TEXT_MAX_LEN);
        return this->repaint();
    }


    bool repaint() {
        tc_mode_set_bg(Color::WHITE, true);
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






template <unsigned NUM_OPT=2>
class Exit_Component : public TUI_Component {

private:
    int selected_option = -1;

    char options[NUM_OPT][EXIT_OPTION_MAX_LEN];


public:
    Exit_Component() {
        for (unsigned i = 0; i < NUM_OPT; i++) {
            snprintf(options[i], EXIT_OPTION_MAX_LEN, "%u", i);
        }
    }

    int get_option() {
        return selected_option;
    }

    void set_option_text(unsigned option, const char* text) {
        assert(option < NUM_OPT);
        strncpy(options[option], text, EXIT_OPTION_MAX_LEN);
    }

    void set_option(int opt) {
        assert(0 <= opt && opt < (int)NUM_OPT);
        selected_option = opt;
        repaint_options();
    }

    bool repaint() {
        assert(bounds.height >= 4);
        tc_mode_set(Mode::NONE, Color::WHITE, true, Color::BLACK);

        for (uint16_t row = bounds.row; row < bounds.row + bounds.height; row++) 
        {
            tc_cursor_set_pos(row, bounds.col); 
            if (row == bounds.row || row == bounds.row + bounds.height - 1) {
                printf("+");
                print_repeated("-", bounds.width-2);
                printf("+");
                continue;
            }
            printf("|");

            if ((row == bounds.row + bounds.height - 2)) 
            {
                // Keep this row clear for options
                tc_cursor_move_column((int16_t)(bounds.width-2));
            } else
            {
                if ((row - bounds.row) == (bounds.height-1)/2) {
                    printf_aligned(bounds.width-2, Align::CENTER, "%s", "Exit App?");
                } else {
                    print_spaces(bounds.width-2);
                }
            }
            printf("|");
        }


        repaint_options();

        tc_mode_reset();

        return true;
    }

    void repaint_options() {
        tc_cursor_set_pos((uint16_t)(bounds.row + bounds.height - 2), bounds.col+1); 

        unsigned width = bounds.width - 2;
        unsigned num_printed = 0;
        unsigned width_per_option = width / NUM_OPT;

        for (unsigned i = 0; i < NUM_OPT; i++) {
            if ((int)i == selected_option) {
                tc_mode_set(Mode::BOLD, Color::BLACK, false, Color::WHITE, true); 
            } else {
                tc_mode_set(Mode::BOLD, Color::WHITE, true, Color::BLACK, false); 
            }
            
            if (i+1 == NUM_OPT)
                width_per_option = width - num_printed; 
            num_printed += (unsigned)printf_aligned(width_per_option, Align::CENTER, "%s", options[i]);
        }
        tc_mode_reset();
    }
};
