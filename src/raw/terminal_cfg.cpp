#include "terminal_cfg.h"

#include <termios.h>
#include <sys/ioctl.h>

#include <assert.h>

static termios stored_terminal_cfg;
static bool is_stored_terminal_cfg_valid = false;

// TODO is fd for tcsetattr 0 or 1??

int terminal_cfg_store()
{
	int ret = tcgetattr(1, &stored_terminal_cfg);
	if (ret != 0)
		return ret;	
	is_stored_terminal_cfg_valid = true;	
	return 0;
}

int terminal_cfg_restore()
{
	if (is_stored_terminal_cfg_valid) {
		return tcsetattr(1, TCSADRAIN, &stored_terminal_cfg);
	}
	return -1;
}


void terminal_cfg_set(bool echo, bool canonical)
{

	termios terminal_cfg = stored_terminal_cfg;

	// En-/Disable local echo
	if (echo) {
		terminal_cfg.c_lflag |= ECHO;
	} else {
	    terminal_cfg.c_lflag &= ~ECHO; 
	}

	// En-/Disable buffered I/O
	if (canonical) {
		terminal_cfg.c_lflag |= ICANON;
	} else {
	    terminal_cfg.c_lflag &= ~ICANON; 
		terminal_cfg.c_cc[VMIN] = 1;
		terminal_cfg.c_cc[VTIME] = 0;
	}

	tcsetattr(1, TCSANOW, &terminal_cfg);
}

int terminal_cfg_get_size(uint16_t *rows, uint16_t *cols) 
{
	struct winsize size;
	assert(-1 != ioctl(1, TIOCGWINSZ, &size));
	*rows = size.ws_row;
	*cols = size.ws_col;

	return 0;
}


