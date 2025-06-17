#include "terminal_traffic_analyzer.h"

#include <stdio.h>
#include <unistd.h>

#include <memory.h>

#include <stdlib.h>

#include "ANSI_Escape_Codes.h"



void TerminalTrafficAnalyzer::capture_input(char data[], unsigned long size) 
{
    unsigned long data_idx = 0;

    for (unsigned long i = 0; i < size; i++) 
    {
        char c = data[i];

        const char *token = "";
        const char *description = "";
        if (parse_esc_code(c, &token, &description)) {
            // Write all previous data
            if (i > data_idx)
                fwrite(data + data_idx, i - data_idx, 1, out_file);
        }

        input_buffer.push_back(c);
        // Check if should wait for next character
        if (!(this->INPUT_WAIT_NEWLINE && c == '\n'))
            continue;

        // TODO use coloring if enabled 
        fprintf(out_file, "[IN]'%s'\n", input_buffer.c_str());
        input_buffer.clear();
    }
}


void TerminalTrafficAnalyzer::capture_output(char data[], unsigned long size) 
{
    unsigned long data_idx = 0;

    for (unsigned long i = 0; i < size; i++) 
    {
        char c = data[i];

        switch (this->state) {
            default:
            case State::TEXT: {
                const char *token = "";
                const char *description = "";
                if (parse_esc_code(c, &token, &description)) {
                    // Write all previous data
                    if (i > data_idx)
                        fwrite(data + data_idx, i - data_idx, 1, out_file);
                    if (c == ESC) {
                        this->state = State::ESC_EXPR;
                    } else {
                        // TODO print token and maybe description
//                        write(this->fd);
                    } 
                    break;
                }
                break;
            }
            case State::ESC_EXPR:
                output_buffer.push_back(c);

                // Check for expression end: letter
                if (isalpha(c)) {
                    parse_expression(output_buffer.c_str(), output_buffer.size());
                    output_buffer.clear();
                    this->state = State::TEXT;
                }

                break;
        }

        if (data_idx < size)
            fwrite(data + data_idx, size - data_idx, 1, out_file);
    }
}

void TerminalTrafficAnalyzer::parse_expression(const char* expr, unsigned long size) 
{
    char type = expr[size-1]; 

    const char *comment = "Unknown expression";

    switch(type) {
        default:
            break;
            
    }

    fprintf(out_file, "[ESC%s %s]\n", expr, comment);
}


bool TerminalTrafficAnalyzer::parse_esc_code(char c, const char **out_token, const char **out_description) {
    switch (c) {
        case ESC:
           *out_token = "ESC";
           *out_description = "Escape";
           break;
        case DEL:
           *out_token = "DEL";
           *out_description = "Delete";
           break;
        case BELL:
           *out_token = "BELL";
           *out_description = "Bell";
           break;
        case BS:
           *out_token = "BS";
           *out_description = "Backspace";
           break;
        case HT:
           *out_token = "HT";
           *out_description = "Horizontal Tab";
           break;
        case VT:
           *out_token = "VT";
           *out_description = "Vertical Tab";
           break;
        case CR:
           *out_token = "CR";
           *out_description = "Carriage Return";
           break;
        case LF:
           *out_token = "LF";
           *out_description = "Linefeed";
           break;
        case FF:
           *out_token = "FF";
           *out_description = "Formfeed";
           break;
        default:
            return false;
    }
    return true;
}
