#include <stdio.h>
#include <string.h>

#include <assert.h>

#include <termios.h>
#include <fcntl.h>

#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#include <string>

#include "terminal_cfg.h"

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
    assert(argc > 0);

    if (argc == 1) {
        printf(
            "Usage: %s [-o file.txt] [-o </dev/pts/XX>] <program> [<args for program>]\n"
            "\t-o output file (default: <program>.txt)\n" 
            "\t-o output to other terminal </dev/pts/XX>; use output of tty command in other terminal\n" 
        ,argv[0]);

        return 0;
    }

    // Iterate through flags; copy up to two filenames
    std::string out_filename;
    std::string out_filename_2;

    int arg_idx = 1;
    while (arg_idx < argc && argv[arg_idx][0] == '-') {
        switch (argv[arg_idx][1]) {
        case 'o':
            arg_idx++;
            assert(arg_idx < argc);

            if (out_filename.empty())
                out_filename = argv[arg_idx];
            else 
                out_filename_2 = argv[arg_idx];
            break;
        default:
            fprintf(stderr, "Ignoring invalid command option '%s'\n", argv[arg_idx]);
        }
        arg_idx++;
    }

    // Assuming that <program> is at arg_idx now
    assert(arg_idx < argc);

    // If no filename given, assign default filename 
    if (out_filename.empty()) {
        out_filename = argv[arg_idx]; 
        out_filename = out_filename.substr(out_filename.find_last_of('/')+1);
        out_filename.append(".txt");
    }

    printf("Output to file '%s'\n", out_filename.c_str());
    if (!out_filename_2.empty())
    printf("\tAnd to file '%s'\n", out_filename_2.c_str());


    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);


    terminal_cfg_store();
    terminal_cfg_set(false, false, true);

    FILE *out_file = nullptr, *out_file_2 = nullptr;
    out_file   = fopen(out_filename.c_str(), "a"); //TODO open in which mode?
    if (!out_filename_2.empty())
        out_file_2 = fopen(out_filename_2.c_str(), "a"); //TODO open in which mode?

    // Disable output buffer for /dev/xx files
    if (out_filename.find_first_of("/dev/") == 0)
        setbuf(out_file, NULL); 
    if (out_filename_2.find_first_of("/dev/") == 0)
        setbuf(out_file_2, NULL); 


    // Create pipes from and to child process
    int pipe_fd_child_stdout[2];
    int pipe_fd_child_stdin[2];

    assert(0 == pipe2(pipe_fd_child_stdout, O_NONBLOCK));
    assert(0 == pipe2(pipe_fd_child_stdin, O_NONBLOCK));

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
        dup2(pipe_fd_child_stdout[1], 1);

        // Child stdout: Close pipe write direction, dup pipe read direction to stdout
        close(pipe_fd_child_stdin[1]);
        dup2(pipe_fd_child_stdin[0], 0);
    
/*
        printf("[Child] Checkpoint\n");

        while (1) {
            int i = getchar();
            if (i != EOF) {
                printf("[Child] Got '%c'\n", (char)i);
            }
        }
*/

        execv(argv[arg_idx], (char* const*)(argv + arg_idx)); //TODO currently requires full path, e.g. /usr/bin/echo instead of echo
        // This should never be reached, unless execv failed
        printf("execv exited with error code %d\n", errno);

        close(pipe_fd_child_stdout[1]);
        close(pipe_fd_child_stdin[0]);
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
    close(pipe_fd_child_stdin[0]);
    int fd_child_stdin = pipe_fd_child_stdin[1];

    // fprintf(stderr, "[Parent] Checkpoint\n");


    while (!sig_child_received)
    {
        int i = getchar();
        if (i != EOF) {
            // fprintf(stderr, "[Parent] getchar '%c', writing to child\n", (char)i);
            char c = (char)i;
            write(fd_child_stdin, &c, 1);
        }

        char c;
        if (0 < read(fd_child_stdout, &c, 1)) {
            // fprintf(stderr, "[Parent] From child: '%c', writing to file\n", c);
            putc(c, out_file);
            if (out_file_2 != nullptr)
                putc(c, out_file_2);
            putc(c, stdout);
        }
    }

//    kill(child_pid, SIGINT)

    printf("=============\n");
    printf("Process ended.\n");
    printf("=============\n");

    terminal_cfg_restore();

    fclose(out_file); 
    if (out_file_2 != nullptr)
        fclose(out_file_2); 

    return 0;
}