#pragma once

#include <map>

// ASCII codes
#define ESC '\x1b'
#define DEL '\x7f'
#define BELL '\a'
#define BS '\b' // Backspace
#define HT '\t' // Horizontal TAB
#define VT '\v' // Vertical TAB
#define CR '\r'
#define LF '\n'
#define FF '\f' // Formfeed; equals new page

#define CODE_ESC "\x1b"
#define CODE_DEL "\x7f"
#define CODE_BELL "\a"
#define CODE_BS "\b" // Backspace
#define CODE_HT "\t" // Horizontal TAB
#define CODE_VT "\v" // Vertical TAB
#define CODE_CR "\r"
#define CODE_LF "\n"
#define CODE_FF "\f" // Formfeed; equals new page

// Cursor Controls
#define ESC_CURSOR_SET_POS_TO_HOME      CODE_ESC"[H"
#define ESC_CURSOR_SET_POS              CODE_ESC"[%u;%uH"
#define ESC_CURSOR_MOVE_UP              CODE_ESC"[%uA"
#define ESC_CURSOR_MOVE_DOWN            CODE_ESC"[%uB"
#define ESC_CURSOR_MOVE_RIGHT           CODE_ESC"[%uC"
#define ESC_CURSOR_MOVE_LEFT            CODE_ESC"[%uD"

#define ESC_CURSOR_MOVE_DOWN_TO_LINE_BEGIN  CODE_ESC"[%uE"
#define ESC_CURSOR_MOVE_UP_TO_LINE_BEGIN    CODE_ESC"[%uF"

#define ESC_CURSOR_SET_COLUMN CODE_ESC"[%uG"

#define ESC_CURSOR_REQUEST_POS CODE_ESC"[6n"

#define ESC_CURSOR_MOVE_UP_ONE          CODE_ESC"M" // Scrolling if needed
#define ESC_CURSOR_SAVE_POS             CODE_ESC"7"    // Using DEC sequence
#define ESC_CURSOR_RESTORE_POS          CODE_ESC"8" // Using DEC sequence

// Erase Functions
#define ESC_ERASE_FROM_CURSOR_TO_SCREEN_END     CODE_ESC"[0J"
#define ESC_ERASE_FROM_CURSOR_TO_SCREEN_BEGIN   CODE_ESC"[1J"
#define ESC_ERASE_ENTIRE_SCREEN                 CODE_ESC"[2J"
#define ESC_ERASE_SAVED_LINES                   CODE_ESC"[3J"
#define ESC_ERASE_FROM_CURSOR_TO_LINE_END       CODE_ESC"[0K"
#define ESC_ERASE_FROM_CURSOR_TO_LINE_BEGIN     CODE_ESC"[1K"
#define ESC_ERASE_ENTIRE_LINE                   CODE_ESC"[2K"

// Color / Graphics Mode
enum class Mode {
	NONE = 0,
	BOLD = 1,
	DIM = 2,
	ITALIC = 3,
	UNDERLINE = 4,
	BLINKING = 5,
	INVERSE = 7,
	HIDDEN = 8,
	STRIKETHROUGH = 9,
	RESET_BOLD = 22 // Listed extra since it's a special case
};

const std::map<Mode, const char*> Mode_Str {
	{Mode::NONE, "Reset"},
	{Mode::BOLD, "Bold"},
	{Mode::DIM, "Dim"},
	{Mode::ITALIC, "Italic"},
	{Mode::UNDERLINE, "Underline"},
	{Mode::BLINKING, "Blinking"},
	{Mode::INVERSE, "Inverse"},
	{Mode::HIDDEN, "Hidden"},
	{Mode::STRIKETHROUGH, "Strikethrough"},
 };


enum class Color {
	BLACK = 0,
	RED = 1,
	GREEN = 2,
	YELLOW = 3,
	BLUE = 4,
	MAGENTA = 5,
	CYAN = 6,
	WHITE = 7,
	DEFAULT = 9
};

const std::map<Color, const char*> Color_Str {
	{Color::BLACK, "Black"},
	{Color::RED, "Red"},
	{Color::GREEN, "Green"},
	{Color::YELLOW, "Yellow"},
	{Color::BLUE, "Blue"},
	{Color::MAGENTA, "Magenta"},
	{Color::CYAN, "Cyan"},
	{Color::WHITE, "White"},
	{Color::DEFAULT, "Default"},
 };





#define ESC_MODE_COLOR_FG_OFFSET 30
#define ESC_MODE_COLOR_BG_OFFSET 40
#define ESC_MODE_COLOR_FG_BRIGHT_OFFSET 90
#define ESC_MODE_COLOR_BG_BRIGHT_OFFSET 100

#define ESC_MODE_RESET_OFFSET 20

#define ESC_MODE                CODE_ESC"[%um"
#define ESC_MODE_COLOR          CODE_ESC"[%u;%um"
#define ESC_MODE_COLOR_FG_BG    CODE_ESC"[%u;%u;%um"

#define ESC_COLOR_FG_256        CODE_ESC"[38;5;%um" // %u being the color ID
#define ESC_COLOR_BG_256        CODE_ESC"[48;5;%um" // %u being the color ID

#define ESC_COLOR_FG_RGB        CODE_ESC"[38;2;%u;%u;%um" // %u %u %u being r g b
#define ESC_COLOR_BG_RGB        CODE_ESC"[38;2;%u;%u;%um" // %u %u %u being r g b

// Common private modes
#define ESC_SCREEN_SAVE         CODE_ESC"[?47l"
#define ESC_SCREEN_RESTORE      CODE_ESC"[?47h"

#define ESC_CURSOR_INVISIBLE_EN     CODE_ESC"[?25l"
#define ESC_CURSOR_INVISIBLE_DIS    CODE_ESC"[?25h"

#define ESC_ALT_BUFFER_EN       CODE_ESC"[?1049h"
#define ESC_ALT_BUFFER_DIS      CODE_ESC"[?1049l"
