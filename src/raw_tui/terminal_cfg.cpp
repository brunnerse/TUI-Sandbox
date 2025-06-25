#include "terminal_cfg.h"

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <assert.h>
#include <stdio.h>

#include <string>


static termios stored_terminal_cfg;
static bool is_stored_terminal_cfg_valid = false;

static int TERMINAL_STDx_FILENO = -1;

// Find the file descriptor fd connected to the terminal 
// If STDIN, STDOUT are not directly connected to a terminal (e.g. because they are piped),
// We might still be able to access the terminal configuration using STDERR 
static int find_terminal_fileno()
{
	int fileno_to_check[3] = {STDERR_FILENO, STDOUT_FILENO, STDIN_FILENO};
	for (unsigned i = 0; i < 3; i++) {
		if (isatty(fileno_to_check[i]))
		{
			TERMINAL_STDx_FILENO = fileno_to_check[i];
			return TERMINAL_STDx_FILENO;
		}
	}
	return -1;
}

bool terminal_is_connected()
{
	if (TERMINAL_STDx_FILENO == -1 && find_terminal_fileno() == -1)
		return false;
	return true;
}

int terminal_cfg_store()
{
	if (TERMINAL_STDx_FILENO == -1 && find_terminal_fileno() == -1)
		return -1;

	int ret = tcgetattr(TERMINAL_STDx_FILENO, &stored_terminal_cfg);
	if (ret != 0)
		return ret;	
	is_stored_terminal_cfg_valid = true;	
	return 0;
}

int terminal_cfg_restore()
{
	if (is_stored_terminal_cfg_valid) {
		return tcsetattr(TERMINAL_STDx_FILENO, TCSADRAIN, &stored_terminal_cfg);
	}
	return -2;
}


int terminal_cfg_set(bool echo, bool canonical, bool input_nonblocking)
{
	int file_status_flags = fcntl(0, F_GETFL);
	fcntl(STDIN_FILENO, F_SETFL, input_nonblocking ? (file_status_flags | O_NONBLOCK) : (file_status_flags & ~O_NONBLOCK));


	if (TERMINAL_STDx_FILENO == -1 && find_terminal_fileno() == -1)
		return -1;

	termios terminal_cfg;
	if (is_stored_terminal_cfg_valid) {
		terminal_cfg = stored_terminal_cfg;
	} else {
		int ret = tcgetattr(TERMINAL_STDx_FILENO, &terminal_cfg);
		if (ret != 0)
			return ret;	
	}

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

	return tcsetattr(TERMINAL_STDx_FILENO, TCSANOW, &terminal_cfg);
}

int terminal_cfg_get_size(uint16_t *rows, uint16_t *cols) 
{
	struct winsize size;
	int ret = ioctl(TERMINAL_STDx_FILENO, TIOCGWINSZ, &size);

	if (ret == -1) {
        fprintf(stderr, "[Error] ioctl exited with error code %d\n", errno);
		assert(-1 != ioctl(1, TIOCGWINSZ, &size));
	}

	*rows = size.ws_row;
	*cols = size.ws_col;

	return 0;
}

