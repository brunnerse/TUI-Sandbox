#include <stdlib.h>
#include <curses.h>
#include <signal.h>

static void finish(int sig);

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;


    int num = 0;

    /* initialize your non-curses data structures here */

    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) echo();         /* echo input - in color */

    if (has_colors())
    {
        start_color();

        /*
         * Simple color assignment, often all we need.  Color pair 0 cannot
         * be redefined.  This example uses the same value for the color
         * pair as for the foreground color, though of course that is not
         * necessary:
         */
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
    }

    for (;;)
    {
        (void) getch();     /* Wait until a single keystroke of input */
        attrset(COLOR_PAIR(num % 8));
        num++;

        /* process the command keystroke */
    }

    finish(0);               /* we are done */
}

static void finish(int sig)
{
    (void)sig;

    endwin();

    /* do your non-curses wrapup here */

    exit(0);
}
