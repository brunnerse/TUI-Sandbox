#include "terminal_traffic_analyzer.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#include <memory.h>
#include <assert.h>

#include <vector>
#include <chrono>

#include "ANSI_Escape_Codes.h"

static uint32_t timestamp_ms()
{
    return (uint32_t)
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void TerminalTrafficAnalyzer::init_pre_suffixes()
{
    
    if (! (this->USE_COLORS && this->PRINT_COLORS_ONLY) ) {
        input_prefix = "{IN '";
        input_suffix = "'}";
	    expression_prefix = "| ";
        expression_suffix = " |";
    }

    description_prefix = " (";
    description_suffix = ")";
	esc_code_prefix = "<"; 
    esc_code_suffix = ">";



    char str[20];
    if (this->USE_COLORS)
    {

        snprintf(str, sizeof(str), ESC_MODE, 
            ESC_MODE_COLOR_FG_OFFSET + (uint8_t)Color::GREEN);
        input_prefix.insert(0,  str);
        snprintf(str, sizeof(str), ESC_MODE, (uint8_t)Mode::NONE);
        input_suffix.append(str);

        snprintf(str, sizeof(str), ESC_MODE_COLOR, (uint8_t)Mode::BOLD, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::BLACK);
        esc_code_prefix.insert(0,  str);
        esc_code_suffix.insert(0, str);
        snprintf(str, sizeof(str), ESC_MODE_COLOR, (uint8_t)Mode::RESET_BOLD, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::DEFAULT);
        esc_code_suffix.append(str);

        snprintf(str, sizeof(str), ESC_MODE_COLOR, (uint8_t)Mode::BOLD, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::BLACK);
        expression_prefix.insert(0,  str);
        expression_suffix.insert(0, str);
        snprintf(str, sizeof(str), ESC_MODE_COLOR "<ESC>", (uint8_t)Mode::BOLD, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::BLACK);
        expression_prefix.append(str);

        snprintf(str, sizeof(str), ESC_MODE_COLOR, (uint8_t)Mode::RESET_BOLD, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::DEFAULT);
        expression_suffix.append(str);


        snprintf(str, sizeof(str), ESC_MODE_COLOR_FG_BG, (uint8_t)Mode::DIM, (uint8_t)Mode::RESET_BOLD,
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::BLACK);
        description_prefix.insert(0,  str);
        snprintf(str, sizeof(str), ESC_MODE_COLOR, ESC_MODE_RESET_OFFSET + (uint8_t)Mode::DIM, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::DEFAULT);
        description_suffix.append(str);
    } 
    else 
    {
        expression_prefix += esc_code_prefix + "ESC" + esc_code_suffix;
    }
}



void TerminalTrafficAnalyzer::capture(char data[], unsigned long size, std::string& buffer, const std::string& prefix, const std::string& suffix)
{
    bool in_esc_expression = !buffer.empty();

    bool append_linefeed = false;

    // If there have been no new capture for some time: Output a linefeed
    uint32_t current_ms = timestamp_ms();
    if (current_ms - last_print_time_ms > OUTPUT_LF_AFTER_MS)
    {
        fputc(LF, out_file);
    }
    last_print_time_ms = current_ms;

    fwrite(prefix.c_str(), prefix.size(), 1, out_file);

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
                buffer += expression_prefix;
                in_esc_expression = true;
            }
            else 
            {
                buffer += esc_code_prefix;
                if (token[0] == '?') {
                    // Escape expression unknown; Add hex value to buffer
                    char s[10];
                    snprintf(s, sizeof(s), "\\x%02x", c);
                    buffer.append(s);
                } else {

                    buffer += token;
                    if (PRINT_ESC_CODE_DESCRIPTIONS)
                    {
                        buffer += description_prefix + description + description_suffix;
                    }
                }
                buffer += esc_code_suffix;

                if (c == LF)
                {
                    append_linefeed = true; 
                }
            }
        } else {
            // If currently in an escape expression: Push to buffer and the expression just ended
            if (in_esc_expression)
            {
                buffer.push_back(c);
                // If expression just ended ; TODO this does not work for special keys like F1, PGUP,...
                if (isalpha(c) && c != 'O')
                {
                    in_esc_expression = false;

                    if (PRINT_EXPRESSION_DESCRIPTIONS)
                    {
                        char expression_desc[100];
                        size_t esc_expression_pos = expression_prefix.size();
                        bool linefeed_after = 
                            parse_expression(buffer.substr(esc_expression_pos).c_str(), 
                                buffer.size() - esc_expression_pos,
                                expression_desc, sizeof(expression_desc));

                        append_linefeed |= linefeed_after;
                    
                        buffer += description_prefix + expression_desc + description_suffix;

                    }

                    buffer.append(expression_suffix);
                }
            }
        }

        if (!in_esc_expression && buffer.size() > 0)
        {
            fwrite(buffer.c_str(), buffer.size(), 1, out_file);
            buffer.clear();
            data_idx = i + 1;

            // If at end of data: Append after suffix later, otherwise append directly
            if (append_linefeed && i < size-1) {
                fputc(LF, out_file);
                append_linefeed = false;
            }
        }
    }
    
    // Write all remaining data if not currently writing to output_buffer
    if (!in_esc_expression) {
        fwrite(data + data_idx, size - data_idx, 1, out_file);
    }
    fwrite(suffix.c_str(), suffix.size(), 1, out_file);
    if (append_linefeed)
        fputc(LF, out_file);
    
}

void TerminalTrafficAnalyzer::capture_input(char data[], unsigned long size) 
{
    this->capture(data, size, input_buffer, input_prefix, input_suffix);
}

void TerminalTrafficAnalyzer::capture_output(char data[], unsigned long size) 
{
    this->capture(data, size, output_buffer, output_prefix, output_suffix);
}


static const char *get_mode_str(Mode mode) 
{
    auto elem_iter = Mode_Str.find(mode);
    if (elem_iter == Mode_Str.end())
        return "?";
    return elem_iter->second;
}

static const char *get_color_str(Color color) 
{

    auto elem_iter = Color_Str.find(color);
    if (elem_iter == Color_Str.end())
        return "?";
    return elem_iter->second;
}

bool TerminalTrafficAnalyzer::parse_expression(const char* expr, size_t size, char* out_description, size_t out_length) 
{
    // Print  
    snprintf(out_description, out_length, "Ill-formed expression");

    char type = expr[size-1]; 

    //fprintf(out_file, "Parsing expr '%s' type %c\n", expr, type);


    if (expr[0] != '[' && expr[0] != 'O') {
        // TODO handle these kind of expressions specially, also in capture() method
        snprintf(out_description, out_length, "Unknown special expression");
        return true;
    }

    const char *expr_nums = &expr[1];

    bool private_mode = false;
    if (expr[1] == '?') {
        private_mode = true;
        expr_nums = &expr[2];
    } 

    // parse any numbers in expression

    // TODO something does not work with number parsing yet
    std::vector<int> expr_numbers;
    int count = 0;
    while (!isalpha(*expr_nums) && *expr_nums != '\0')
    {
//        fprintf(out_file, "At %s in expr %s\n", expr_nums, expr);
        count++;
        if (count > 10) // Ill-formed expression, or something went wrong
           return true; 
        int num = atoi(expr_nums);
        if (num < 0) {
            // ill-formed
            return true;
        }
        expr_numbers.push_back(num);
        while (isdigit(*expr_nums) && *expr_nums != '\0')
            expr_nums++;
        if (*expr_nums == ';')
            expr_nums++;
 //       fprintf(out_file, "[%u]", expr_numbers.back());
    }


    if (private_mode)
    {
        if (expr_numbers.empty()) {
            fprintf(out_file, "Ill-formed expression!\n");
            return true;
        }
        switch(expr_numbers[0]) {
            case 25:
                if (type == 'l')
                    snprintf(out_description, out_length, "Make cursor invisible"); 
                else if (type == 'h')
                    snprintf(out_description, out_length, "Make cursor visible"); 
                break;
            case 47:
                if (type == 'l')
                    snprintf(out_description, out_length, "Restore screen");
                else if (type == 'h')
                    snprintf(out_description, out_length, "Save screen"); 
                break;
            case 1049:
                if (type == 'l')
                    snprintf(out_description, out_length, "Disable alternative buffer");
                else if (type == 'h')
                    snprintf(out_description, out_length, "Enable alternative buffer"); 
                break;
            default:
                snprintf(out_description, out_length, "Unknown private expression"); 
                break;
        }
        return true;
    }


    switch(type) {
        case 'm': {
            // Empty is equal to 0
            if (expr_numbers.empty())
                expr_numbers.push_back(0);

            int len = snprintf(out_description, out_length, "Mode:");

            for (int num : expr_numbers) {
                if (num <= (int)Mode::STRIKETHROUGH) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " %s;", 
                        get_mode_str((Mode)num));
                }
                else if (num >= ESC_MODE_RESET_OFFSET && num <= ESC_MODE_RESET_OFFSET + (int)Mode::STRIKETHROUGH) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " reset %s;", 
                        get_mode_str((Mode)(num - ESC_MODE_RESET_OFFSET)));
                }
                else if (num >= ESC_MODE_COLOR_FG_OFFSET && num <= ESC_MODE_COLOR_FG_OFFSET+ (int)Color::DEFAULT) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " Color %s;", 
                        get_color_str((Color)(num - ESC_MODE_COLOR_FG_OFFSET)));
                }
                else if (num >= ESC_MODE_COLOR_BG_OFFSET && num <= ESC_MODE_COLOR_BG_OFFSET+ (int)Color::DEFAULT) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " Bg-Color %s;", 
                        get_color_str((Color)(num - ESC_MODE_COLOR_BG_OFFSET)));
                }

                else 
                    len += snprintf(out_description+len, out_length-(uint32_t)len, "?;");
            }

            return false;
            break;
        }
        case 'H':
            if (expr_numbers.size() == 2)
                snprintf(out_description, out_length, "Move cursor to line %u col %u", expr_numbers[0], expr_numbers[1]);
            else
                snprintf(out_description, out_length, "Move cursor to home (line 0, col 0)");
            break;
        case 'J': {
            // Erase in display
            const char *text = "entire";
            if (expr_numbers.size() > 0 && expr_numbers[0] == 1)
                text = "from cursor until end of";
            else if (expr_numbers.size() > 0 && expr_numbers[0] == 2)
                text = "from cursor to beginning of";
            snprintf(out_description, out_length, "Erase %s display", text);
            break;
        }
        case 'K': {
            // Erase in line
            const char *text = "entire";
            if (expr_numbers.size() > 0 && expr_numbers[0] == 1)
                text = "from cursor until end of";
            else if (expr_numbers.size() > 0 && expr_numbers[0] == 2)
                text = "from cursor to beginning of";
            snprintf(out_description, out_length, "Erase %s line", text);
            break;
        }
        case 'D':
            if (expr_numbers.size() > 0) {
                snprintf(out_description, out_length, "Move cursor left %u columns", expr_numbers[0]);
                return true;
            } else {
                snprintf(out_description, out_length, "LEFT ARROW");
                return false;
            }
            break;
        case 'C':
            if (expr_numbers.size() > 0) {
                snprintf(out_description, out_length, "Move cursor right %u columns", expr_numbers[0]);
                return true;
            } else {
                snprintf(out_description, out_length, "RIGHT ARROW");
                return false;
            }
            break;
        case 'A':
            if (expr_numbers.size() > 0) {
                snprintf(out_description, out_length, "Move cursor up %u lines", expr_numbers[0]);
                return true;
            } else {
                snprintf(out_description, out_length, "UP ARROW");
                return false;
            }
            break;
        case 'B':
            if (expr_numbers.size() > 0) {
                return true;
                snprintf(out_description, out_length, "Move cursor down %u lines", expr_numbers[0]);
            } else {
                snprintf(out_description, out_length, "DOWN ARROW");
                return false;
            }
            break;
        default:
            snprintf(out_description, out_length, "Unknown expression");
            break;
    }
    return true;
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
        case '\x0e':
           *out_token = "SO";
           *out_description = "Shift out";
            break;
        case '\x0f':
           *out_token = "SI";
           *out_description = "Shift in";
            break;
        default:
            if (c >= ' ' && c <= '~')
                return false;
            *out_token = "?";
           *out_description = "unknown";
    }
    return true;
}
