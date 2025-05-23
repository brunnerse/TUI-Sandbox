#pragma once



// ASCII codes
#define ESC "\x1b"
#define DEL "\x7f"
#define CODE_BELL "\a"
#define CODE_BS "\b" // Backspace
#define CODE_HT "\t" // Horizontal TAB
#define CODE_VT "\v" // Vertical TAB
#define CODE_CR "\r"
#define CODE_LF "\n"
#define CODE_FF "\f" // Formfeed; equals new page

// Cursor Controls
#define ESC_CURSOR_SET_POS_TO_HOME ESC"[H"
#define ESC_CURSOR_SET_POS ESC"[%u;%uH" 
#define ESC_CURSOR_MOVE_UP ESC"[%uA"
#define ESC_CURSOR_MOVE_DOWN ESC"[%uB"
#define ESC_CURSOR_MOVE_RIGHT ESC"[%uC"
#define ESC_CURSOR_MOVE_LEFT ESC"[%uD"

#define ESC_CURSOR_MOVE_DOWN_TO_LINE_BEGIN ESC"[%uE"
#define ESC_CURSOR_MOVE_UP_TO_LINE_BEGIN ESC"[%uF"

#define ESC_CURSOR_SET_COLUMN ESC"[%uG" 

#define ESC_CURSOR_REQUEST_POS ESC"[6n" 

#define ESC_CURSOR_MOVE_UP_ONE ESC"M"  // Scrolling if needed
#define ESC_CURSOR_SAVE_POS ESC"7"  // Using DEC sequence
#define ESC_CURSOR_RESTORE_POS ESC"8" // Using DEC sequence


// Erase Functions
#define ESC_ERASE_FROM_CURSOR_TO_SCREEN_END ESC"[0J" 
#define ESC_ERASE_FROM_CURSOR_TO_SCREEN_BEGIN ESC"[1J" 
#define ESC_ERASE_ENTIRE_SCREEN ESC"[2J" 
#define ESC_ERASE_SAVED_LINES ESC"[3J" 
#define ESC_ERASE_FROM_CURSOR_TO_LINE_BEGIN ESC"[0K" 
#define ESC_ERASE_FROM_CURSOR_TO_LINE_START ESC"[1K" 
#define ESC_ERASE_ENTIRE_LINE ESC"[2K" 



// Color / Graphics Mode
// TODO define color / mode offsets here aswell?

#define ESC_MODE_COLOR_FG_OFFSET 30
#define ESC_MODE_COLOR_BG_OFFSET 40
#define ESC_MODE_COLOR_FG_BRIGHT_OFFSET 90
#define ESC_MODE_COLOR_BG_BRIGHT_OFFSET 100

#define ESC_MODE_RESET_OFFSET 21

#define ESC_MODE ESC"[%um"

#define ESC_COLOR_FG_256 ESC"[38;5;%um" // %u being the color ID
#define ESC_COLOR_BG_256 ESC"[48;5;%um" // %u being the color ID

#define ESC_COLOR_FG_RGB ESC"[38;2;%u;%u;%um" // %u %u %u being r g b 
#define ESC_COLOR_BG_RGB ESC"[38;2;%u;%u;%um" // %u %u %u being r g b 





// Common private modes
#define ESC_SCREEN_SAVE ESC "[?47l"
#define ESC_SCREEN_RESTORE ESC "[?47h"

#define ESC_CURSOR_INVISIBLE_EN ESC "[?25l"
#define ESC_CURSOR_INVISIBLE_DIS ESC "[?25h"

#define ESC_ALT_BUFFER_EN ESC "[?1049h"
#define ESC_ALT_BUFFER_DIS ESC "[?1049l"
