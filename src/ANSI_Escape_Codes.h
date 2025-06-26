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
#define SI '\xe' // Shift in
#define SO '\xf' // Shift out 

#define CODE_ESC "\x1b"
#define CODE_DEL "\x7f"
#define CODE_BELL "\a"
#define CODE_BS "\b" // Backspace
#define CODE_HT "\t" // Horizontal TAB
#define CODE_VT "\v" // Vertical TAB
#define CODE_CR "\r"
#define CODE_LF "\n"
#define CODE_FF "\f" // Formfeed; equals new page
#define CODE_SI "\xe" // Shift in
#define CODE_SO "\xf" // Shift out 


// Cursor Controls
#define ESC_CURSOR_SET_POS_TO_HOME      CODE_ESC"[H"
#define ESC_CURSOR_SET_POS              CODE_ESC"[%u;%uH"
#define ESC_CURSOR_SET_ROW              CODE_ESC"[%ud"
#define ESC_CURSOR_SET_COLUMN   		CODE_ESC"[%uG"
#define ESC_CURSOR_MOVE_UP              CODE_ESC"[%uA"
#define ESC_CURSOR_MOVE_DOWN            CODE_ESC"[%uB"
#define ESC_CURSOR_MOVE_RIGHT           CODE_ESC"[%uC"
#define ESC_CURSOR_MOVE_LEFT            CODE_ESC"[%uD"

#define ESC_CURSOR_MOVE_DOWN_TO_LINE_BEGIN  CODE_ESC"[%uE"
#define ESC_CURSOR_MOVE_UP_TO_LINE_BEGIN    CODE_ESC"[%uF"

#define ESC_CURSOR_REQUEST_POS 			CODE_ESC"[6n"

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
#define ESC_ERASE_CHARACTERS                   CODE_ESC"[%uX"

#define ESC_INSERT_LINES 						CODE_ESC"[%uL"
#define ESC_REMOVE_LINES 						CODE_ESC"[%uM"

#define ESC_SET_VIEWPORT_TOP_BOTTOM_MARGIN 		CODE_ESC"[%u;%ur"

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


#define ESC_CHARSET_SWITCH_TO_ACS 		CODE_ESC"(0"
#define ESC_CHARSET_SWITCH_TO_ASCII 	CODE_ESC"(B"

// Alternative Character Set ACS
// Overview: https://c-for-dummies.com/ncurses/tables/table04-03.php

#define ACS_ULCORNER	"l" /* upper left corner */
#define ACS_LLCORNER	"m" /* lower left corner */
#define ACS_URCORNER	"k" /* upper right corner */
#define ACS_LRCORNER	"j" /* lower right corner */
#define ACS_LTEE	"t" /* tee pointing right */
#define ACS_RTEE	"u" /* tee pointing left */
#define ACS_BTEE	"v" /* tee pointing up */
#define ACS_TTEE	"w" /* tee pointing down */
#define ACS_HLINE	"q" /* horizontal line */
#define ACS_VLINE	"x" /* vertical line */
#define ACS_PLUS	"n" /* large plus or crossover */
#define ACS_S1		"o" /* scan line 1 */
#define ACS_S9		"s" /* scan line 9 */
#define ACS_DIAMOND	"\x60"  /* diamond */
#define ACS_CKBOARD	"a" /* checker board (stipple) */
#define ACS_DEGREE	"f" /* degree symbol */
#define ACS_PLMINUS	"g" /* plus/minus */
#define ACS_BULLET	"~" /* bullet */
/* Teletype 5410v1 symbols begin here */
#define ACS_LARROW	"," /* arrow pointing left */
#define ACS_RARROW	"+" /* arrow pointing right */
#define ACS_DARROW	"." /* arrow pointing down */
#define ACS_UARROW	"^" /* arrow pointing up */
#define ACS_BOARD	"h" /* board of squares */
#define ACS_LANTERN	"i" /* lantern symbol */
#define ACS_BLOCK	"0" /* solid square block */
/*
 * These aren't documented, but a lot of System Vs have them anyway
 * (you can spot pprryyzz{{||}} in a lot of AT&T terminfo strings).
 * The ACS_names may not match AT&T's, our source didn't know them.
 */
#define ACS_S3		"p" /* scan line 3 */
#define ACS_S7		"r" /* scan line 7 */
#define ACS_LEQUAL	"y" /* less/equal */
#define ACS_GEQUAL	"z" /* greater/equal */
#define ACS_PI		"{" /* Pi */
#define ACS_NEQUAL	"|" /* not equal */
#define ACS_STERLING	"}" /* UK pound sign */
