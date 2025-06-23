#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "MoreLess_App.h"

void exit_with_usage(char *argv[])
{
    printf(
        "Usage: %s <file.txt> [-c] [-a]\n"
        "<file.txt>: file to display\n"
        "\t[-a] use alternate display mode\n" 
        "\t[-c] color output\n",
        argv[0]
    );
    exit(0);
}



int main(int argc, char *argv[])
{
    if (argc < 2)
        exit_with_usage(argv);

    const char *file = nullptr; 
    bool use_colors = false;
    bool use_alt_screen = false;

    for (int arg_idx = 1; arg_idx < argc; arg_idx++) 
    {
        if (argv[arg_idx][0] == '-') { //flag
            switch (argv[arg_idx][1]) {
            case 'c':
                use_colors = true;
                break;
            case 'a':
                use_alt_screen = true;
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


    MoreLess_App app(file, use_alt_screen, use_colors);

    int ret = app.start();

    assert(0 == ret);
    return 1;
}

