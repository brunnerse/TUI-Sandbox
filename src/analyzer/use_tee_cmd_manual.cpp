#include <stdio.h>
#include <string.h>

#include <assert.h>

#include <termios.h>
#include <fcntl.h>

#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>

#include <string>

#include "terminal_cfg.h"
#include "terminal_traffic_analyzer.h"

#include "tcdebug_arg_parse.h"


#define INTERCEPT_INPUT 1


volatile bool sig_int_received = false;

void sigint_handler(int arg) {
    (void)arg;
    sig_int_received = true;
}


int main(int argc, char **argv)
{

    tcdebug_args args;
    if (!tcdebug_parse_args(argc, argv, false, false, false, &args))
        return 1;

    FILE *out_file;
    if (!args.out_files[0].empty()) 
        out_file = fopen(args.out_files[0].c_str(), "w");
    else 
        out_file = stdout;

    // Disable output buffer for /dev/xx files 
    if (args.out_files[0].find_first_of("/dev/") == 0)
        setbuf(out_file, NULL); 

    signal(SIGINT, sigint_handler);

    terminal_cfg_store();
    terminal_cfg_set(false, false, false);



    // Create input and output pipes
    int pipe_fd_program_input[2];
    assert(0 == pipe2(pipe_fd_program_input, 0));
    int pipe_fd_program_output[2];
    assert(0 == pipe2(pipe_fd_program_output, 0));

    pid_t own_pid = getpid();
    printf("Process has PID %lu\n", (unsigned long)own_pid);

    // Both pipes read: Close write direction, use read direction
    close(pipe_fd_program_input[1]);
    int fd_program_input = pipe_fd_program_input[0];

    close(pipe_fd_program_output[1]);
    int fd_program_output = pipe_fd_program_output[0];

    printf("====== Setup ======\n");
    printf("Type in other terminal:\n");
//    printf("cat $(tty) | tee /proc/%u/fd/%u | %s | tee /proc/%u/fd/%u\n",
    printf("tee /proc/%u/fd/%u | %s | tee /proc/%u/fd/%u\n",
        own_pid, fd_program_input, 
        (args.program != nullptr) ? args.program : "<program>", 
        own_pid, fd_program_output);
    printf("======\n");

    TerminalTrafficAnalyzer analyzer(out_file);

    while (!sig_int_received)
    {
        char buf[PIPE_BUF+1];

        ssize_t nbytes = read(fd_program_input, &buf, PIPE_BUF);
        if (nbytes > 0) {
            analyzer.capture_input(buf, (unsigned long)nbytes);
            buf[nbytes+1] = '\0';
            printf("[IN] %s\n", buf);
        }

        nbytes = read(fd_program_output, &buf, PIPE_BUF);
        if (nbytes > 0) {
            analyzer.capture_output(buf, (unsigned long)nbytes);
            buf[nbytes+1] = '\0';
            printf("[OUT] %s\n", buf);
        }
    }

    printf("==================\n");
    printf("Stopped.\n");
    printf("==================\n");

    terminal_cfg_restore();

    if (out_file != stdout)
        fclose(out_file);

    return 0;
}