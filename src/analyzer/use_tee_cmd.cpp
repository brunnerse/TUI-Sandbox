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
    if (!tcdebug_parse_args(argc, argv, false, true, false, &args))
        return 1;

    if (args.out_files[0].empty())
            args.out_files[0] = get_default_out_filename(&args);

    print_parsed_args_info(&args);

    signal(SIGINT, sigint_handler);

    terminal_cfg_store();
    terminal_cfg_set(false, false, false);

    FILE *out_file = nullptr;
    out_file   = fopen(args.out_files[0].c_str(), "w");


    // Create input and output pipes
    int pipe_fd_program_input[2];
    assert(0 == pipe2(pipe_fd_program_input, O_NONBLOCK));
    int pipe_fd_program_output[2];
    assert(0 == pipe2(pipe_fd_program_output, O_NONBLOCK));

    pid_t parent_pid = getpid();
    printf("Process has PID %lu\n", (unsigned long)parent_pid);

    // Both pipes read: Close write direction, use read direction
    close(pipe_fd_program_input[1]);
    int fd_program_input = pipe_fd_program_input[0];

    close(pipe_fd_program_output[1]);
    int fd_program_output = pipe_fd_program_output[0];


    pid_t child_pid = fork();
    if (child_pid == 0) {
        printf("[Child] Child has PID %lu\n", (unsigned long)getpid());
        printf("%u %s\n", args.program_argc, args.program_argv[0]);
        std::string program_call = args.program_argv[0]; 
        for (int i = 1; i < args.program_argc; i++) {
            program_call += " "; 
            program_call += args.program_argv[i];
        }
        // Make sure no pipes are in the program call
        assert (program_call.find('|') == std::string::npos);

        char cmd[200];
        int retval = snprintf(cmd, sizeof(cmd), "tee /proc/%u/fd/%u | %s | tee /proc/%u/fd/%u\n",
            parent_pid, fd_program_input, 
            program_call.c_str(),
            parent_pid, fd_program_output);
        assert(retval > 0 && retval < (int)sizeof(cmd)); 

        printf("[Child] Executing %s\n", cmd);
        printf("Press key to start...");
        getchar(); 
        execl("/bin/sh", "sh", "-c", cmd, (char*)nullptr);
    }


    TerminalTrafficAnalyzer analyzer(out_file);

    while (!sig_int_received)
    {
        char buf[PIPE_BUF+1];

        ssize_t nbytes = read(fd_program_input, &buf, PIPE_BUF);
        if (nbytes > 0) {
            //analyzer.capture_input(buf, (unsigned long)nbytes);
            buf[nbytes+1] = '\0';
            fprintf(out_file, "[IN] %s\n", buf);
        }

        nbytes = read(fd_program_output, &buf, PIPE_BUF);
        if (nbytes > 0) {
            //analyzer.capture_output(buf, (unsigned long)nbytes);
            buf[nbytes+1] = '\0';
            fprintf(out_file, "[OUT] %s\n", buf);
        }
    }


    terminal_cfg_restore();

    printf("==================\r\n");
    printf("Stopped.\r\n");
    printf("==================\r\n");

    if (out_file != stdout)
        fclose(out_file);


    return 0;
}