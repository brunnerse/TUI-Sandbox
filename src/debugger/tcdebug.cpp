#include <stdio.h>
#include <string.h>

#include <assert.h>

#include <termios.h>
#include <fcntl.h>

#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#include <string>

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
            "\t-o output to other terminal <dev/pts/XX>; use output of tty command in other terminal\n" 
        ,argv[0]);

        return 0;
    }

    // TODO handle multiple filenames

    // Iterate through flags
    std::string out_filename("");

    int arg_idx = 1;
    while (arg_idx < argc && argv[arg_idx][0] == '-') {
        switch (argv[arg_idx][1]) {
        case 'o':
            arg_idx++;
            assert(arg_idx < argc);
            out_filename = argv[arg_idx];
            break;
        default:
            fprintf(stderr, "Ignoring invalid command option '%s'\n", argv[arg_idx]);
        }
        arg_idx++;
    }

    // Assuming that <program> is at arg_idx now
    assert(arg_idx < argc);

    if (out_filename.size() == 0) {
        out_filename = argv[arg_idx]; 
        out_filename = out_filename.substr(out_filename.find_last_of('/')+1);
        out_filename.append(".txt");
    }

    printf("Output to file %s\n", out_filename.c_str());


    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    int pipefd_parent2child[2];
    int pipefd_child2parent[2];

    assert(0 == pipe(pipefd_parent2child));
    assert(0 == pipe(pipefd_child2parent));

    // TODO execute program in new process, forward stdio
    pid_t child_pid = fork();

    if (child_pid == 0) {
        puts("Child entering.\n");

        // Inside child process
        // Close read direction for child2parent, and write direction for parent2child
        close(pipefd_child2parent[0]);
        close(pipefd_parent2child[1]);

        printf("=============\n");
        printf("Executing '%s",argv[arg_idx]);
        for (int idx = arg_idx+1; idx < argc; idx++)
            printf(" %s", argv[idx]);
        printf("'\n");
        printf("=============\n");

        dup2(pipefd_child2parent[1], 1); // Duplicate child2parent to stdout
        dup2(pipefd_parent2child[0], 0); // Duplicate parent2child to stdin

        execv(argv[arg_idx], (char* const*)(argv + arg_idx)); //TODO currently requires full path, e.g. /usr/bin/echo instead of echo
        // This should never be reached, unless execv failed
        printf("execv exited with error code %d\n", errno);

        close(pipefd_child2parent[1]);
        close(pipefd_parent2child[0]);
        return -1; 
    }

    if (child_pid == -1) {
        fprintf(stderr, "Failed to fork process\n");
        return 1;
    }

    FILE* outfile = fopen(out_filename.c_str(), "a"); //TODO open in which mode?

    printf("Child has PID %lu\n", (unsigned long)child_pid);


    // Close read direction for parent2child, and write direction for child2parent
    close(pipefd_child2parent[1]);
    close(pipefd_parent2child[0]);

    int fd_from_child = pipefd_child2parent[0];
    int fd_to_child = pipefd_parent2child[1];


    // Todo can I use terminal_cfg() functions here??

    // Prepare stdin: Set non-canonical, non-blocking 
    fcntl(0, F_SETFL, O_NONBLOCK);

    // TODO do  I need to do this for fd 1 aswell / instead?

    struct termios term_settings; 
    tcgetattr(0, &term_settings); /* grab old terminal i/o settings */

    struct termios old_term_settings = term_settings; 
    term_settings.c_lflag &= (unsigned)~ICANON; /* disable buffered i/o */
    term_settings.c_lflag &= (unsigned)~ECHO; /* disable echo mode */
    term_settings.c_cc[VMIN] = 1;

    tcsetattr(0, TCSANOW, &term_settings); /* use these new terminal i/o settings now */

    while (!sig_child_received)
    {
        char c;
        if (0 < read(fd_from_child, &c, 1)) {
            // Write character to file
            // TODO Handle escape sequences, and special characters like \r, \f
            //fprintf(outfile, "[%c]", c);
            putc(c, outfile);

            // Forward character to terminal stdout
            //printf("%c|", c);
            //putc(c, stdout);
            write(1, &c, 1);
        }


        // TODO might not be necessary
        if (0 < read(0, &c, 1)){
            printf("Read from 0!");
            write(fd_to_child, &c, 1);
        }
        //sleep(1);
        //c = 'A';
        //write(fd_to_child, &c, 1);
    }

//    kill(child_pid, SIGINT)
    printf("=============\n");
    printf("Process ended.\n");
    printf("=============\n");

    tcsetattr(0, TCSANOW, &old_term_settings); /* restore old terminal i/o settings now */
    fclose(outfile); 
    return 0;
}