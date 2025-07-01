#include <stdio.h>
#include <assert.h>

#include <stdlib.h>

#include "Snake.h"

void exit_with_usage(char *argv[])
{
    printf(
        "Usage: %s" 
        "\t[-a] use alternate display mode\n"
        "\t[-b] use borders\n",
        argv[0]
    );
    exit(0);
}



int main(int argc, char *argv[])
{
    bool use_alt_screen = false;
    bool use_borders = false;

    for (int arg_idx = 1; arg_idx < argc; arg_idx++) 
    {
        if (argv[arg_idx][0] == '-') { //flag
            int flag_idx = 1;
            do {
                switch (argv[arg_idx][flag_idx]) {
                case 'a':
                    use_alt_screen = true;
                    break;
                case 'b':
                    use_borders = true;
                    break;
                default:
                    exit_with_usage(argv); 
                }
                flag_idx++;
            } while (argv[arg_idx][flag_idx] != '\0');
        }
        else 
        {
            exit_with_usage(argv);
        }
    }

    Snake app(use_alt_screen, use_borders);

    int ret = app.start();

    assert(0 == ret);

    return 0;
}

