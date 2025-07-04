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

#include <errno.h>

#include "terminal_cfg.h"
#include "terminal_traffic_analyzer.h"

#include "tcdebug_arg_parse.h"

#define INTERCEPT_INPUT true
#define NUM_OUTPUT_FILES 1


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


int main(int argc, char **argv)
{

    tcdebug_args args;
    if (!tcdebug_parse_args(argc, argv, NUM_OUTPUT_FILES != 1, true, true, &args))
        return 1;

    if (args.out_files[0].empty())
            args.out_files[0] = get_default_out_filename(&args);
        
    print_parsed_args_info(&args);


    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);


    terminal_cfg_store();
    terminal_cfg_set(false, false, true);

    FILE *out_files[NUM_OUTPUT_FILES];
    unsigned num_output_files = 0;
    for (unsigned i = 0; i < NUM_OUTPUT_FILES; i++)
    {
        if (args.out_files[i].empty())
            break;

        out_files[i] = fopen(args.out_files[i].c_str(), "w");
        assert(out_files[i] != nullptr);
        num_output_files++;

        // Disable output buffer for /dev/xx files 
        if (args.out_files[i].find_first_of("/dev/") == 0)
            setbuf(out_files[i], NULL); 
    }

    // Disable stdout buffer
    setbuf(stdout, NULL);

    // Open fd to null in case we need to throw away some data from a pipe
    int fd_null = open("/dev/null", O_WRONLY);


    // Create pipes from and to child process
    int pipe_fd_child_stdout[2];
    assert(0 == pipe2(pipe_fd_child_stdout, O_NONBLOCK));

#if INTERCEPT_INPUT
    int pipe_fd_child_stdin[2];
    assert(0 == pipe2(pipe_fd_child_stdin, 0));
#endif

    pid_t parent_pid = getpid();
    pid_t child_pid = fork();

    if (child_pid == 0)
    {
        // Inside child process
        printf("\nPress key to start...");
        while (getchar() == EOF)
            ; 
        printf("\r\e[2K\n");
       
        // Signal parent that child will start the program now
        kill(parent_pid, SIGINT);


        // Child stdout: Close pipe read direction, dup pipe write direction to stdout
        close(pipe_fd_child_stdout[0]);
        dup2(pipe_fd_child_stdout[1], STDOUT_FILENO);

        // Child stdin: Close pipe write direction, dup pipe read direction to stdin
#if INTERCEPT_INPUT
        close(pipe_fd_child_stdin[1]);
        dup2(pipe_fd_child_stdin[0], STDIN_FILENO);
#endif
        
    
        execvp(args.program_argv[0], (char* const*)args.program_argv); 

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

    //printf("Child has PID %lu\n", (unsigned long)child_pid);

    // Read child stdout: Close pipe write direction, use pipe read direction
    close(pipe_fd_child_stdout[1]);
    int fd_child_stdout = pipe_fd_child_stdout[0];

    // Write child stdin: Close pipe read direction, use pipe write direction
#if INTERCEPT_INPUT
    close(pipe_fd_child_stdin[0]);
    int fd_child_stdin = pipe_fd_child_stdin[1];
#endif

    TerminalTrafficAnalyzer analyzer(out_files[0]);


    // Wait until child sends signal that it started program 
    while (!sig_int_received)
        sleep(100);
    
    // Continuously read 
    while (!sig_child_received)
    {
        char buf[PIPE_BUF];
        ssize_t nbytes; 
#if INTERCEPT_INPUT
        nbytes = read(STDIN_FILENO, &buf, PIPE_BUF);
        if (nbytes > 0) {
            write(fd_child_stdin, buf, (size_t)nbytes);
            analyzer.capture_input(buf, (size_t)nbytes);
        }
#endif
        nbytes = read(fd_child_stdout, &buf, PIPE_BUF);
        if (nbytes > 0)
        {
            if (args.delay_after_esc_expr_ms > 0) 
            {
                bool user_interrupted = false;

                size_t idx_tail = 0;
                size_t idx_head = 0;

                // Search for first esc and skip it (as we do not want to wait before it)
                while (buf[idx_tail] != ESC && idx_tail + 1 < (size_t)nbytes)
                    idx_tail++;

                while (idx_tail < (size_t)nbytes) 
                {
                    idx_tail++;
                    // If caught start of esc expression or end of buf:
                    // Process everything up until ESC (excluding the ESC)
                    if (buf[idx_tail] == ESC || idx_tail == (size_t)nbytes) 
                    {
                        // Calc sub-buffer dimensions, update head
                        char *buf_start = &buf[idx_head];
                        size_t size = idx_tail - idx_head;
                        idx_head = idx_tail;

                        write(STDOUT_FILENO, buf_start, size);
                        analyzer.capture_output(buf_start, size);

                        if (!user_interrupted) { 
                            // Wait for specified ms; if user sends an input character, during, abort
                            const uint32_t ms_per_loop = 10;
                            for (uint32_t i = 0; i < args.delay_after_esc_expr_ms / ms_per_loop; i++) 
                            {
                                usleep(1000 * ms_per_loop);
                                char c[5];
                                ssize_t size = read(STDIN_FILENO, c, sizeof(c));
                                if (size > 0) {
                                    user_interrupted = true;
                                    write(fd_child_stdin, c, (size_t)size);
                                    analyzer.capture_input(c, (size_t)size);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else {
                write(STDOUT_FILENO, buf, (size_t)nbytes);
                analyzer.capture_output(buf, (size_t)nbytes);

            }
        }
    }

//    kill(child_pid, SIGINT)


    terminal_cfg_restore();

    printf("\n==================\n");
    printf("Program finished.\n");
    printf("==================\n");


    close(fd_null);

    for (unsigned i = 0; i < num_output_files; i++)
        fclose(out_files[i]); 

    return 0;
}
