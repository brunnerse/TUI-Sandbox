#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <signal.h>

#include <curses.h>

#define USER_ECHO false
#define NONBLOCKING_GETCH true

bool sigint_received = false;

void sigint_handler(int arg) 
{
    (void)arg;
    fprintf(stderr, "SIGINT received\n");
    sigint_received = true;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    signal(SIGINT, sigint_handler);

    initscr();            /* Start curses mode 		*/

    raw();                /* Line buffering disabled	*/
    //cbreak();           /* Or instead: Enable program signal handling*/

    keypad(stdscr, TRUE); /* We get F1, F2 etc..		*/
    if (USER_ECHO)
        echo();
    else
        noecho();             /* Don't echo() while we do getch */

    if (NONBLOCKING_GETCH)
        halfdelay(5);          /* Make getch() return after 5/10th of a second*/

    printw("Type any character to see it in bold\n");
    while (!sigint_received) {
        int ch = getch();             /* If raw() hadn't been called
                                    * we have to press enter before it
                                    * gets to the program 		*/
        if (ch == EOF) 
            continue;
        if (ch == KEY_F(1)) {          /* Without keypad enabled this will */
            printw("F1 Key pressed"); /*  not get to us either	*/
        /* Without noecho() some ugly escape
        * charachters might have been printed
        * on screen			*/
        }
        else
        {
            printw("The pressed key is ");
            attron(A_BOLD);
            printw("%c", ch);
            attroff(A_BOLD);
            printw("\t'\\%02x'", ch);
            printw("\n");
        }
        refresh(); /* Print it on to the real screen */
    }


    endwin();  /* End curses mode		  */

    return 0;
}
