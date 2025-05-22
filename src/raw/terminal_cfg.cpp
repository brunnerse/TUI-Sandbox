#include "terminal_cfg.h"

#include <termios.h>
#include <sys/ioctl.h>

#include <assert.h>

static termios stored_terminal_cfg;
static bool is_stored_terminal_cfg_valid = false;



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
		return tcsetattr(1, 0, &stored_terminal_cfg);
	}
	return -1;
}


void terminal_cfg_set(bool echo, bool canonical)
{

	termios terminal_cfg = stored_terminal_cfg;

	if (echo) {

	} else {

	}

	if (canonical) {

	} else {

	}


	tcsetattr(1, 0, &terminal_cfg);
}

int terminal_cfg_get_size(uint16_t *cols, uint16_t *rows) 
{
	struct winsize size;
	assert(-1 != ioctl(1, TIOCGWINSZ, &size));
	*cols = size.ws_col;
	*rows = size.ws_row;

	return 0;
}


