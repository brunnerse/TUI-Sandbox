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
    if (!tcdebug_parse_args(argc, argv, false, true, &args))
        return 1;
    print_parsed_args_info(&args);


    FILE *out_file;
    if (!args.out_files[0].empty()) 
        out_file = fopen(args.out_files[0].c_str(), "w");
    else 
        out_file = stdout;

    // Disable buffer to output file TODO necessary?
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



//    pid_t child_pid = fork();
//    if (child_pid != 0) {
//        printf("[Child] Child has PID %lu\n", (unsigned long)child_pid);
        char cmd[100];
        int retval = snprintf(cmd, 100, "cat $(tty) | tee /proc/%u/fd/%u | %s | tee /proc/%u/fd/%u\n",
            own_pid, fd_program_input, 
            args.program,
            own_pid, fd_program_output);
        assert(retval > 0 && retval < 100); 

        printf("[Child] Executing %s", cmd);

        
        execl("/bin/sh", "sh", "-c", cmd, (char*)nullptr);

        //char* const sh_argv[3] = {"sh" "-c", cmd};
//        execvp(sh_argv[0], sh_argv);
//        exit(system(args.program));
 //   }


    TerminalTrafficAnalyzer analyzer(out_file);

    // TODO ncurses program does not get my input
    while (!sig_int_received)
    {
        char buf[PIPE_BUF+1];


        ssize_t nbytes = read(fd_program_input, &buf, PIPE_BUF);
        if (nbytes > 0) {
            //analyzer.capture_input(buf, (unsigned long)nbytes);
            buf[nbytes+1] = '\0';
            printf("[IN] %s\n", buf);
        }

        nbytes = read(fd_program_output, &buf, PIPE_BUF);
        if (nbytes > 0) {
            //analyzer.capture_output(buf, (unsigned long)nbytes);
            buf[nbytes+1] = '\0';
            printf("[OUT] %s\n", buf);
        }
    }

    printf("==================\n");
    printf("Stopped.\n");
    printf("==================\n");

    if (out_file != stdout)
        fclose(out_file);

    terminal_cfg_restore();

    return 0;
}