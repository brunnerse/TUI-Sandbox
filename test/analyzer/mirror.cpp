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


#define DEBUG_STDERR 0

#define HEARTBEAT 1
#define HEARTBEAT_PERIOD_SEC 2


volatile bool sig_int_received = false;

void sigint_handler(int arg) {
    (void)arg;

    sig_int_received = true;

    terminal_cfg_restore();

    printf("[MIRROR] Exiting.\n");
        
    exit(0);
}


int main()
{
    signal(SIGINT, sigint_handler);

    time_t heartbeat_time = time(NULL);

    terminal_cfg_store();
    terminal_cfg_set(false, false, true);

    setbuf(stdout, NULL);

    bool in_escape_expression = false;

    while (!sig_int_received)
    {
#if HEARTBEAT
        time_t now = time(NULL);
        if (now - heartbeat_time > HEARTBEAT_PERIOD_SEC) {
            printf("\n[Mirror Heartbeat at %lu]\n", heartbeat_time);
            heartbeat_time = now;
        }
#endif
        int c = getchar();
        if (c == EOF)
            continue;
        
        int old_c = c;        

//        if (islower(c))
//            c = toupper(c);

#if DEBUG_STDERR
        fprintf(stderr, "[Mirror] From stdin: '%c', to stdout: '%c'\n", old_c, c);
#else
        (void)old_c;
#endif

        if (!in_escape_expression) {
            if (c == '\e') {
                in_escape_expression = true;
                printf("From stdin: '%c", c);
            }
            else 
                printf("From stdin: '%c'", c);
        }
        else 
        {
            if (isalpha(c)) {
                in_escape_expression = false;
                printf("%c'\n", c);
            }
            else 
            {
                printf("%c", c);
            }
        }

    }

    return 0;
}