#include "terminal_cfg.h"

#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <assert.h>
#include <stdio.h>

#include <string>


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


void terminal_cfg_set(bool echo, bool canonical, bool input_nonblocking)
{
	int file_status_flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL, input_nonblocking ? (file_status_flags | O_NONBLOCK) : (file_status_flags & ~O_NONBLOCK));

	//TODO do it need setattr for 1 or 0 ? Are they the same ???

	termios terminal_cfg = stored_terminal_cfg;

	// En-/Disable local echo
	if (echo) {
		terminal_cfg.c_lflag |= ECHO;
	} else {
	    terminal_cfg.c_lflag &= (unsigned)~ECHO; 
	}

	// En-/Disable buffered I/O
	if (canonical) {
		terminal_cfg.c_lflag |= ICANON;
	} else {
	    terminal_cfg.c_lflag &= (unsigned)~ICANON; 
		terminal_cfg.c_cc[VMIN] = 1;
		terminal_cfg.c_cc[VTIME] = 0;
	}

	tcsetattr(1, TCSANOW, &terminal_cfg);
}

int terminal_cfg_get_size(uint16_t *rows, uint16_t *cols) 
{
	struct winsize size;
	int ret = ioctl(1, TIOCGWINSZ, &size);

	if (ret == -1) {
        fprintf(stderr, "[Error] ioctl exited with error code %d\n", errno);
		assert(-1 != ioctl(1, TIOCGWINSZ, &size));
	}

	*rows = size.ws_row;
	*cols = size.ws_col;

	return 0;
}

