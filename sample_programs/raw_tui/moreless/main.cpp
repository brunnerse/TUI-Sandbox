#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "MoreLess_App.h"

void exit_with_usage(char *argv[])
{
    printf(
        "Usage: %s <file.txt> [-a] [-i]\n"
        "<file.txt>: file to display\n"
        "\t[-a] use alternate display mode\n"
        "\t[-i] ignore if not directly connected to a terminal\n", 
        argv[0]
    );
    exit(0);
}



int main(int argc, char *argv[])
{
    if (argc < 2)
        exit_with_usage(argv);

    // TODO behave like cat if not a argv flag given

    const char *file = nullptr; 
    bool use_alt_screen = false;
    bool check_if_output_not_tty = true;

    for (int arg_idx = 1; arg_idx < argc; arg_idx++) 
    {
        if (argv[arg_idx][0] == '-') { //flag
            switch (argv[arg_idx][1]) {
            case 'a':
                use_alt_screen = true;
                break;
            case 'i':
                check_if_output_not_tty = false;
                break;
            default:
                exit_with_usage(argv); 
            }
        }
        else 
        {
            if (file != nullptr)
                exit_with_usage(argv);
            file = argv[arg_idx];
        }
    }

    if (file == nullptr)
        exit_with_usage(argv);


    MoreLess_App app(file, use_alt_screen, check_if_output_not_tty);

    return app.start();
}

