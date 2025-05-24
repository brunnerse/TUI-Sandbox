#include <stdio.h>
#include <string.h>

#include <assert.h>

#include <termios.h>
#include <fcntl.h>
#include <time.h>

#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#include <string>

#include "terminal_cfg.h"


#define WRITE_TO_FILE_IMMEDIATELY true
#define DEBUG_STDERR 0

#define HEARTBEAT 1
#define HEARTBEAT_PERIOD_SEC 3


FILE *outfile = nullptr;

volatile bool sig_int_received = false;

void sigint_handler(int arg) {
    (void)arg;

    sig_int_received = true;

    terminal_cfg_restore();

    printf("[MIRROR] Exiting.\n");

    if (outfile != nullptr)
        fclose(outfile); 
        
    exit(0);
}


int main(int argc, char **argv)
{
    if (argc != 2) {
        printf(
            "Usage: %s file.txt\n", argv[0]);
        return 0;
    }

    signal(SIGINT, sigint_handler);

    time_t heartbeat_time = time(NULL);

    // Iterate through flags
    std::string out_filename(argv[1]);
    printf("[MIRROR] Output to file %s\n", out_filename.c_str());

    outfile = fopen(out_filename.c_str(), "a"); 

    terminal_cfg_store();
    terminal_cfg_set(false, false, true);

    setbuf(stdout, NULL);

    if (WRITE_TO_FILE_IMMEDIATELY)
        setbuf(outfile, NULL);

    while (!sig_int_received)
    {
#if HEARTBEAT
        time_t now = time(NULL);
        if (now - heartbeat_time > HEARTBEAT_PERIOD_SEC) {
            printf("\n[Mirror HB]\n");
            heartbeat_time = now;
        }
#endif
        int c = getchar();
        if (c == EOF)
            continue;

        int old_c = c;        

        if ('a' <= c)
            c = c - 'a' + 'A';

#if DEBUG_STDERR
        fprintf(stderr, "[Mirror] From stdin: '%c', to stdout: '%c'\n", old_c, c);
#else
        (void)old_c;
#endif

        putc(c, outfile);
        putc(c, stdout);
    }

    return 0;
}