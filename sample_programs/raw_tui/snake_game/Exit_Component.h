#pragma once

#include <string.h>

#include "TUI_App.h"

#include "tc.h"
#include "print_helper.h"

#define EXIT_OPTION_MAX_LEN 20

template <unsigned NUM_OPT=2>
class Exit_Component : public TUI_Component {

private:
    unsigned selected_option = NUM_OPT+1; // Select invalid option initially

    char options[NUM_OPT][EXIT_OPTION_MAX_LEN];

    const char *text;

public:
    Exit_Component(const char *text) : text(text) {
        for (unsigned i = 0; i < NUM_OPT; i++) {
            snprintf(options[i], EXIT_OPTION_MAX_LEN, "%u", i);
        }
    }

    Exit_Component() : Exit_Component("Exit App?") {}

    unsigned get_selected_option() {
        return selected_option;
    }

    unsigned get_num_options() const {
        return NUM_OPT; 
    }

    void set_option_text(unsigned option, const char* text) {
        assert(option < NUM_OPT);
        strncpy(options[option], text, EXIT_OPTION_MAX_LEN);
    }

    void select_option(unsigned opt) {
        assert(opt < NUM_OPT);
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
                tc_print_repeated('-', bounds.width-2);
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
                    printf_aligned(bounds.width-2, Align::CENTER, "%s", text);
                } else {
                    tc_print_repeated(' ', bounds.width-2);
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
            if (i == selected_option) {
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
