#include <stdio.h>
#include <assert.h>

#include "Example_App.h"



int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Basic_App app;

    int ret = app.start();

    assert(0 == ret);

    printf("App ended.\n");

    return 1;
}

