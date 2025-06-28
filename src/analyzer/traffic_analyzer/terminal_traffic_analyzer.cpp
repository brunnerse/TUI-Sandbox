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
        fd_input.prefix = "{IN '";
        fd_input.suffix = "'}";
	    expression_prefix = "| ";
        expression_suffix = " |";
        acs_prefix = "[ACS \"";
        acs_suffix = "\"]";
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
        fd_input.prefix.insert(0,  str);
        snprintf(str, sizeof(str), ESC_MODE, (uint8_t)Mode::NONE);
        fd_input.suffix.append(str);

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

        snprintf(str, sizeof(str), ESC_MODE_COLOR, (uint8_t)Mode::ITALIC, 
            ESC_MODE_COLOR_BG_BRIGHT_OFFSET + (uint8_t)Color::BLACK);
        acs_prefix.insert(0,  str);
        snprintf(str, sizeof(str), ESC_MODE_COLOR, ESC_MODE_RESET_OFFSET + (uint8_t)Mode::ITALIC, 
            ESC_MODE_COLOR_BG_OFFSET + (uint8_t)Color::DEFAULT);
        acs_suffix.append(str);
    } 
    else 
    {
        expression_prefix += esc_code_prefix + "ESC" + esc_code_suffix;
    }
}

void TerminalTrafficAnalyzer::print_data(const char data[], size_t length, bool is_acs)
{
    if (length == 0)
        return;
    if (!is_acs) {
        fwrite(data, length, 1, out_file);
    }
    else {

        fwrite(acs_prefix.c_str(), acs_prefix.size(), 1, out_file);
        fwrite(data, length, 1, out_file);
        if (USE_COLORS) {
            fprintf(out_file, "\"" ESC_MODE "(" ESC_CHARSET_SWITCH_TO_ACS, (uint8_t)Mode::DIM);
            fwrite(data, length, 1, out_file);
            fprintf(out_file, ESC_CHARSET_SWITCH_TO_ASCII ")" ESC_MODE "]" ESC_MODE, 
                ESC_MODE_RESET_OFFSET + (uint8_t)Mode::DIM, ESC_MODE_RESET_OFFSET + (uint8_t)Mode::ITALIC);
        }
        else 
            fwrite(acs_suffix.c_str(), acs_suffix.size(), 1, out_file);
    }

}


void TerminalTrafficAnalyzer::capture(char data[], unsigned long size, fd_state *fd_x)
{
    bool in_esc_expression = !fd_x->buffer.empty();

    bool append_linefeed = false;

    // If there have been no new capture for some time: Output a linefeed
    uint32_t current_ms = timestamp_ms();
    if (current_ms - last_print_time_ms > OUTPUT_LF_AFTER_MS)
    {
        fputc(LF, out_file);
    }
    last_print_time_ms = current_ms;

    fwrite(fd_x->prefix.c_str(), fd_x->prefix.size(), 1, out_file);

    unsigned long data_idx = 0;

    for (unsigned long i = 0; i < size; i++) 
    {
        char c = data[i];

        const char *token = "";
        const char *description = "";

        bool is_esc_code = parse_esc_code(c, &token, &description);

        if (in_esc_expression)
        {
            // Check if expression already ended one sign before
            bool expression_ended = isspace(c) || is_esc_code;
            if (expression_ended) // If expression already ended: read the character again in the next iteration
                i--;
            else // Else: Character still part of escape expression 
                fd_x->buffer.push_back(c);

            // If expression just ended or ends with this character 
            if (expression_ended || (isalpha(c) && c != 'O') || fd_x->buffer.find("(0") != std::string::npos)
            {
                in_esc_expression = false;

                if (PRINT_EXPRESSION_DESCRIPTIONS)
                {
                    char expression_desc[100];
                    size_t esc_expression_pos = expression_prefix.size();
                    std::string expression = fd_x->buffer.substr(esc_expression_pos); 

                    bool linefeed_after = 
                        parse_expression(expression.c_str(), expression.size(),
                            expression_desc, sizeof(expression_desc));

                    append_linefeed |= linefeed_after;
                
                    fd_x->buffer += description_prefix + expression_desc + description_suffix;

                    if (expression.compare(ESC_CHARSET_SWITCH_TO_ACS+1) == 0)
                        fd_x->is_in_acs = true;
                    else if (expression.compare(ESC_CHARSET_SWITCH_TO_ASCII+1) == 0)
                        fd_x->is_in_acs = false;
                }

                fd_x->buffer.append(expression_suffix);
            }
        }
        else 
        {
            if (is_esc_code)
            {
                // Write all data so far
                print_data(data + data_idx, i - data_idx, fd_x->is_in_acs);

                if (c == ESC && !(fd_x->single_escapes_possible && i == size-1))
                {
                    fd_x->buffer += expression_prefix;
                    in_esc_expression = true;
                }
                else 
                {
                    fd_x->buffer += esc_code_prefix;
                    if (token[0] == '?') {
                        // Escape expression unknown; Add hex value to buffer
                        char s[10];
                        snprintf(s, sizeof(s), "\\x%02x", c);
                        fd_x->buffer.append(s);
                    } else {

                        fd_x->buffer += token;
                        if (PRINT_ESC_CODE_DESCRIPTIONS)
                        {
                            fd_x->buffer += description_prefix + description + description_suffix;
                        }
                    }
                    fd_x->buffer += esc_code_suffix;

                    if (c == LF || c == CR || c == FF || c == VT)
                    {
                        append_linefeed = true; 
                    }
                }
            }
        }

        if (!in_esc_expression && fd_x->buffer.size() > 0)
        {
            fwrite(fd_x->buffer.c_str(), fd_x->buffer.size(), 1, out_file);
            fd_x->buffer.clear();
            data_idx = i + 1;

            // If at end of data: Append after suffix later, otherwise append directly
            if (append_linefeed && i < size-1) {
                fputc(LF, out_file);
                append_linefeed = false;
            }
        }
    }
    
    // Write all remaining data if not currently writing to buffer
    if (!in_esc_expression) {
        print_data(data + data_idx, size - data_idx, fd_x->is_in_acs);
    }
    fwrite(fd_x->suffix.c_str(), fd_x->suffix.size(), 1, out_file);
    if (append_linefeed)
        fputc(LF, out_file);
}

void TerminalTrafficAnalyzer::capture_input(char data[], unsigned long size) 
{
    this->capture(data, size, &fd_input); 
}

void TerminalTrafficAnalyzer::capture_output(char data[], unsigned long size) 
{
    this->capture(data, size, &fd_output); 
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

/* 
*    @return whether to insert a newline after the expression
*/
bool TerminalTrafficAnalyzer::parse_expression(const char* expr, size_t size, char* out_description, size_t out_length) 
{
    // Print  
    snprintf(out_description, out_length, "Ill-formed expression");

    char type = expr[size-1]; 

    //fprintf(out_file, "Parsing expr '%s' type %c\n", expr, type);

    if (expr[0] == '(') {
        switch(expr[1]) {
            case 'B':
                snprintf(out_description, out_length, "Set G0 (default character set) to ASCII");
                break;
            case '0':
                snprintf(out_description, out_length, "Set G0 (default character set) to ACS (for line-drawing)");
                break;
            default:
                snprintf(out_description, out_length, "Unknown G0 character set selection");
        }
        return true;
    }

    if (expr[0] != '[' && expr[0] != 'O') {
        // TODO handle these kind of expressions specially, also in capture() method
        snprintf(out_description, out_length, "Unknown special expression");
        return true;
    }

    const char *expr_nums = &expr[ isalnum(expr[1]) ? 1 : 2];


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


    if (expr[1] == '?') // Private mode expression
    {
        if (expr_numbers.empty()) {
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
    else if (expr[1] == '<')
    {
        // TODO more exact
        snprintf(out_description, out_length, "Mouse input");
        return true;
    }


    // Parse CSI (Control Sequence Introducer) sequences starting with "<ESC>["
    switch(type) {
        case 'm': {
            // Empty is equal to 0
            if (expr_numbers.empty())
                expr_numbers.push_back(0);

            int len = snprintf(out_description, out_length, "Mode:");

            // Special cases
            if (expr_numbers[0] == 38 || expr_numbers[0] == 48) {
                if (expr_numbers[1] == 5) {
                    if (expr_numbers.size() != 3)
                        return false;
                    snprintf(out_description, out_length, "Set %s color to 256-ID %u", 
                        expr_numbers[0] == 38 ? "foreground" : "background", expr_numbers[2]);
                    return false;
                }
                else if (expr_numbers[1] == 2) {
                    if (expr_numbers.size() != 5)
                        return false;
                    snprintf(out_description, out_length, "Set %s color to RGB (%u, %u, %u)", 
                        expr_numbers[0] == 38 ? "foreground" : "background", 
                        expr_numbers[2], expr_numbers[3], expr_numbers[4]);
                    return false;
                }
                else {
                    snprintf(out_description, out_length, "Unknown mode coloring expression");
                    return false;
                }

            }

            bool first = true;
            for (int num : expr_numbers) {
                if (!first)
                    len += snprintf(out_description+len, out_length-(uint32_t)len, ";");
                else
                    first = false;

                if (num <= (int)Mode::STRIKETHROUGH) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " %s", 
                        get_mode_str((Mode)num));
                }
                else if (num >= ESC_MODE_RESET_OFFSET && num <= ESC_MODE_RESET_OFFSET + (int)Mode::STRIKETHROUGH) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " reset %s", 
                        get_mode_str((Mode)(num - ESC_MODE_RESET_OFFSET)));
                }
                else if (num >= ESC_MODE_COLOR_FG_OFFSET && num <= ESC_MODE_COLOR_FG_OFFSET+ (int)Color::DEFAULT) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " Color %s", 
                        get_color_str((Color)(num - ESC_MODE_COLOR_FG_OFFSET)));
                }
                else if (num >= ESC_MODE_COLOR_BG_OFFSET && num <= ESC_MODE_COLOR_BG_OFFSET+ (int)Color::DEFAULT) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " Bg-Color %s", 
                        get_color_str((Color)(num - ESC_MODE_COLOR_BG_OFFSET)));
                }
                else if (num >= ESC_MODE_COLOR_FG_BRIGHT_OFFSET && num <= ESC_MODE_COLOR_FG_BRIGHT_OFFSET+ (int)Color::DEFAULT) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " Color Bright %s", 
                        get_color_str((Color)(num - ESC_MODE_COLOR_FG_BRIGHT_OFFSET)));
                }
                else if (num >= ESC_MODE_COLOR_BG_BRIGHT_OFFSET && num <= ESC_MODE_COLOR_BG_BRIGHT_OFFSET+ (int)Color::DEFAULT) {
                    len += snprintf(out_description+len, out_length-(uint32_t)len, " Bg-Color Bright %s", 
                        get_color_str((Color)(num - ESC_MODE_COLOR_BG_BRIGHT_OFFSET)));
                }
                else 
                    len += snprintf(out_description+len, out_length-(uint32_t)len, "?");
            }

            return false;
            break;
        }
        case 'b': {
            snprintf(out_description, out_length, "Print previous character %u times", 
                expr_numbers.empty() ? 1 : expr_numbers[0]);
            break;
        }
        case 'H':
        case 'f':
            if (expr_numbers.size() == 2)
                snprintf(out_description, out_length, "Move cursor to line %u col %u", expr_numbers[0], expr_numbers[1]);
            else
                snprintf(out_description, out_length, "Move cursor to home (line 0, col 0)");
            break;
        case 'J': 
        case 'K': {
            // Erase in display / line
            const char *text = "entire";
            if (expr_numbers.size() > 0 && expr_numbers[0] == 0)
                text = "from cursor until end of";
            else if (expr_numbers.size() > 0 && expr_numbers[0] == 1)
                text = "from cursor to beginning of";
            snprintf(out_description, out_length, "Erase %s %s", text, type == 'J' ? "display" : "line");
            break;
        }
        case 'X': {
            // Erase in line
            snprintf(out_description, out_length, "Erase %u characters", 
                expr_numbers.empty() ? 1 : expr_numbers[0]);
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
        case 'C':
        case 'a':
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
        case 'B':
        case 'e':
            if (expr_numbers.size() > 0) {
                snprintf(out_description, out_length, "Move cursor down %u lines", expr_numbers[0]);
            } else {
                snprintf(out_description, out_length, "DOWN ARROW");
                return false;
            }
            return true;
        case 'G':
        case '`':
            snprintf(out_description, out_length, "Move cursor to column %u", expr_numbers.empty() ? 1 : expr_numbers[0]);
            break;
        case 'd':
            snprintf(out_description, out_length, "Move cursor to row %u", expr_numbers.empty() ? 1 : expr_numbers[0]);
            break;
        case 'L':
            if (expr_numbers.size() == 0)
                expr_numbers.push_back(1); // Default value
            snprintf(out_description, out_length, "Insert %u blank lines at active row", expr_numbers[0]);
            return true;
        case 'M':
            if (expr_numbers.size() == 0)
                expr_numbers.push_back(1); // Default value
            snprintf(out_description, out_length, "Delete %u blank lines at active row", expr_numbers[0]);
            return true;
        case 'r':
            if (expr_numbers.size() != 2)
                return true;
            snprintf(out_description, out_length, "Set viewport margins to [top=%u, bottom=%u]", expr_numbers[0], expr_numbers[1]);
            return true;
        case 'n':
            if (expr_numbers.size() != 1)
                return true;
            if (expr_numbers[0] == 6)
                snprintf(out_description, out_length, "Request current cursor position");
            else
                snprintf(out_description, out_length, "Unknown expression");
            return true;
        case 'R':
            if (expr_numbers.size() != 2)
                return true;
            snprintf(out_description, out_length, "Response to request: Current cursor position is [row=%u, col=%u]",
                     expr_numbers[0], expr_numbers[1]);
            return true;
        case 's':
            if (!expr_numbers.empty())
                snprintf(out_description, out_length, "Ill-formed expression");
            else
                snprintf(out_description, out_length, "Save cursor position");
            break;
        case 'u':
            if (!expr_numbers.empty())
                snprintf(out_description, out_length, "Ill-formed expression");
            else
                snprintf(out_description, out_length, "Restore cursor position");
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
