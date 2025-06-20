#include "terminal_traffic_analyzer.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#include <memory.h>

#include <chrono>

#include "ANSI_Escape_Codes.h"

static uint32_t timestamp_ms()
{
    return (uint32_t)
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void TerminalTrafficAnalyzer::init_pre_suffixes()
{
    
    if (!this->PRINT_COLORS_ONLY) {
        input_prefix = "{IN '";
        input_suffix = "'}";
    }

	esc_code_prefix = "<"; 
    esc_code_suffix = ">";
	expression_prefix = "[";
    expression_suffix = "]";


    if (this->USE_COLORS)
    {
        char str[20];

        snprintf(str, sizeof(str), ESC_MODE, 
            ESC_MODE_COLOR_FG_OFFSET + (uint8_t)Color::GREEN);
        input_prefix.insert(0,  str);
        snprintf(str, sizeof(str), ESC_MODE, (uint8_t)Mode::NONE);
        input_suffix.append(str);
        snprintf(str, sizeof(str), ESC_MODE_COLOR, (uint8_t)Mode::BOLD, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::BLACK);
        esc_code_prefix.insert(0,  str);
        snprintf(str, sizeof(str), ESC_MODE_COLOR, (uint8_t)Mode::NONE, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::DEFAULT);
        esc_code_suffix.append(str);
        snprintf(str, sizeof(str), ESC_MODE_COLOR, (uint8_t)Mode::ITALIC, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::MAGENTA);
        expression_prefix.insert(0, str);
        snprintf(str, sizeof(str), ESC_MODE_COLOR, (uint8_t)Mode::NONE, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::DEFAULT);
        expression_suffix.append(str);
    } 
    expression_prefix.append(esc_code_prefix);
    expression_prefix.append("ESC");
    expression_prefix.append(esc_code_suffix);

}

void TerminalTrafficAnalyzer::capture_input(char data[], unsigned long size) 
{
    static bool in_esc_expression = false;
    static size_t esc_expression_pos;

    // Print newline first if some time has passed
    uint32_t current_ms = timestamp_ms();
    if (current_ms - last_print_time_ms > OUTPUT_LF_AFTER_MS)
    {
        fwrite(CODE_LF, sizeof(CODE_LF), 1, out_file);
        last_print_time_ms = current_ms;
    }

    for (unsigned long i = 0; i < size; i++) 
    {
        char c = data[i];

        const char *token = "";
        const char *description = "";

        if (parse_esc_code(c, &token, &description)) {
            if (c == ESC)
            {
                input_buffer += expression_prefix;
                in_esc_expression = true;
                esc_expression_pos = input_buffer.size(); 
            }
            else 
            {
                input_buffer += esc_code_prefix;
                if (token[0] == '?') {
                    // Escape expression unknown; Add hex value to buffer
                    char s[10];
                    snprintf(s, sizeof(s), "\\x%02x", c);
                    input_buffer.append(s);
                } else {
                    input_buffer.append(token);
                    output_buffer.append(" ("); 
                    output_buffer.append(description);
                    output_buffer.append(" )"); 
                }
                input_buffer += esc_code_suffix;
                input_buffer += input_prefix;

                if (c == LF)
                {
                    input_buffer.push_back(LF);
                }
            }
        } else {
            input_buffer.push_back(c);

            // Check if currently in an escape expression and the expression just ended
            if (in_esc_expression && isalpha(c))
            {
                in_esc_expression = false;

                char expression_desc[20];
                parse_expression(input_buffer.substr(esc_expression_pos).c_str(), 
                    input_buffer.size() - esc_expression_pos,
                    expression_desc, sizeof(expression_desc));
                input_buffer.append(" ("); 
                input_buffer.append(expression_desc);
                input_buffer.append(" )"); 
                input_buffer.append(expression_suffix);
            }
        }
    }

    if (!input_buffer.empty())
    {
        fprintf(out_file, "%s%s%s", 
            input_prefix.c_str(), input_buffer.c_str(), input_suffix.c_str());
        input_buffer.clear();
    }
}


void TerminalTrafficAnalyzer::capture_output(char data[], unsigned long size) 
{
    static bool in_esc_expression = false;
    static size_t esc_expression_pos;

    uint32_t current_ms = timestamp_ms();
    if (current_ms - last_print_time_ms > OUTPUT_LF_AFTER_MS)
    {
        fwrite(CODE_LF, sizeof(CODE_LF), 1, out_file);
        last_print_time_ms = current_ms;
    }

    unsigned long data_idx = 0;

    for (unsigned long i = 0; i < size; i++) 
    {
        char c = data[i];

        const char *token = "";
        const char *description = "";
        if (parse_esc_code(c, &token, &description)) {

            // Write all data so far
            fwrite(data + data_idx, i - data_idx, 1, out_file);

            if (c == ESC)
            {
                output_buffer += expression_prefix;
                in_esc_expression = true;
                esc_expression_pos = output_buffer.size(); 
            }
            else 
            {
                output_buffer += esc_code_prefix;
                if (token[0] == '?') {
                    // Escape expression unknown; Add hex value to buffer
                    char s[10];
                    snprintf(s, sizeof(s), "\\x%02x", c);
                    output_buffer.append(s);
                } else {
                    output_buffer.append(token);
                    output_buffer.append(" ("); 
                    output_buffer.append(description);
                    output_buffer.append(" )"); 
                }
                output_buffer += esc_code_suffix;

                // TODO newline also in other situations, e.g. after some time?
                if (c == LF)
                {
                    output_buffer.push_back(LF);
                }
            }
        } else {
            // If currently in an escape expression: Push to buffer and the expression just ended
            if (in_esc_expression)
            {
                output_buffer.push_back(c);
                // If expression just ended
                if (isalpha(c))
                {
                    in_esc_expression = false;

                    char expression_desc[20];
                    parse_expression(output_buffer.substr(esc_expression_pos).c_str(), 
                        input_buffer.size() - esc_expression_pos,
                        expression_desc, sizeof(expression_desc));
                    output_buffer.append(" (");
                    output_buffer.append(expression_desc);
                    output_buffer.append(")");
                    output_buffer.append(expression_suffix);
                }
            }
        }

        if (!in_esc_expression && output_buffer.size() > 0)
        {
            fprintf(out_file, "%s", output_buffer.c_str());
            output_buffer.clear();
            data_idx = i + 1;
        }
    }
    
    // Write all remaining data if not currently writing to output_buffer
    if (!in_esc_expression)
        fwrite(data + data_idx, size - data_idx, 1, out_file);
}

void TerminalTrafficAnalyzer::parse_expression(const char* expr, size_t size, char* out_description, size_t out_length) 
{
    char type = expr[size-1]; 

    switch(type) {
        case 'H':
            snprintf(out_description, out_length, "Move cursor to");
            break;
        case 'J':

        default:
            snprintf(out_description, out_length, "Unknown expression");
            break;
            
    }
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
            if (c >= ' ' && c <= '~')
                return false;
            *out_token = "?";
           *out_description = "unknown";
    }
    return true;
}
