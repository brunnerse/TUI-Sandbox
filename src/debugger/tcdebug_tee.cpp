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

// Uses tee() to intercept process input and output
// https://www.man7.org/linux/man-pages/man2/tee.2.html

volatile bool sig_int_received = false;
volatile bool sig_child_received = false;

void sigint_handler(int arg) {
    (void)arg;
    sig_int_received = true;
}

void sigchld_handler(int arg) {
    (void)arg;
    sig_child_received = true;
}

// TODO it is basically impossible to fake being a terminal, maybe try with SSH 


int main(int argc, char **argv)
{

    tcdebug_args args;
    if (!tcdebug_parse_args(argc, argv, false, true, &args))
        return 1;
    print_parsed_args_info(&args);

    std::string out_filename = (!args.out_files[0].empty() ? 
        args.out_files[0] : get_default_out_filename(args));


    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);


    terminal_cfg_store();
    terminal_cfg_set(false, false, true);

    FILE *out_file = nullptr, *out_file_2 = nullptr;
    out_file   = fopen(out_filename.c_str(), "a"); //TODO open in append or write mode?

    // Disable output buffer for /dev/xx files
    if (out_filename.find_first_of("/dev/") == 0)
        setbuf(out_file, NULL); 
    // Disable stdout buffer
    setbuf(stdout, NULL);


    // Create pipes from and to child process
    int pipe_fd_child_stdout[2];
    assert(0 == pipe2(pipe_fd_child_stdout, 0));
#if INTERCEPT_INPUT
    int pipe_fd_child_stdin[2];
    assert(0 == pipe2(pipe_fd_child_stdin, 0));
#endif

    pid_t child_pid = fork();

    if (child_pid == 0)
    {
        // Inside child process
        printf("[Child] =============\n");
        printf("[Child] Executing '%s",argv[arg_idx]);
        for (int idx = arg_idx+1; idx < argc; idx++)
            printf(" %s", argv[idx]);
        printf("'\n");
        printf("[Child] =============\n");

        // Child stdout: Close pipe read direction, dup pipe write direction to stdout
        close(pipe_fd_child_stdout[0]);
        dup2(pipe_fd_child_stdout[1], STDOUT_FILENO);

        // Child stdout: Close pipe write direction, dup pipe read direction to stdout
#if INTERCEPT_INPUT
        close(pipe_fd_child_stdin[1]);
        dup2(pipe_fd_child_stdin[0], STDIN_FILENO);
#endif
    
/*
        printf("[Child] Checkpoint\n");

        while (1) {
            int i = getchar();
            if (i != EOF) {
                printf("[Child] Got '%c'\n", (char)i);
            }
        }
*/
        execvp(argv[arg_idx], (char* const*)(argv + arg_idx)); 

        // This should never be reached, unless execv failed
        fprintf(stderr, "[Error] execv exited with error code %d\n", errno);
        
        close(pipe_fd_child_stdout[1]);
#if INTERCEPT_INPUT
        close(pipe_fd_child_stdin[0]);
#endif
        return -1; 
    }

    if (child_pid == -1) {
        fprintf(stderr, "Failed to fork process\n");
        return 1;
    }


    printf("Child has PID %lu\n", (unsigned long)child_pid);

    // Read child stdout: Close pipe write direction, use pipe read direction
    close(pipe_fd_child_stdout[1]);
    int fd_child_stdout = pipe_fd_child_stdout[0];

    // Write child stdin: Close pipe read direction, use pipe write direction
#if INTERCEPT_INPUT
    close(pipe_fd_child_stdin[0]);
    int fd_child_stdin = pipe_fd_child_stdin[1];
#endif

    TerminalTrafficAnalyzer analyzer(STDERR_FILENO);

    // fprintf(stderr, "[Parent] Checkpoint\n");

    while (!sig_child_received)
    {
        char buf[PIPE_BUF];
        ssize_t nbytes; 
#if INTERCEPT_INPUT
        nbytes = tee(STDIN_FILENO, fd_child_stdin, PIPE_BUF, SPLICE_F_NONBLOCK);
        (void)nbytes;
        nbytes = read(fd_child_stdout, &buf, PIPE_BUF);
        if (nbytes > 0) {
            analyzer.capture_input(buf, nbytes);
        }
#endif
        nbytes = tee(fd_child_stdout, STDOUT_FILENO, PIPE_BUF, SPLICE_F_NONBLOCK);
        (void)nbytes;
        nbytes = read(fd_child_stdout, &buf, PIPE_BUF);
        if (nbytes > 0) {
            analyzer.capture_output(buf, nbytes);
        }
    }

//    kill(child_pid, SIGINT)

    printf("==================\n");
    printf("Child process finished.\n");
    printf("==================\n");

    terminal_cfg_restore();

    fclose(out_file); 
    if (out_file_2 != nullptr)
        fclose(out_file_2); 

    return 0;
}
