#include <stdio.h>
#include <assert.h>

#include "app_basic.h"



int main(int argc, char *argv[])
{

    Basic_App app;

    int ret = app.start();

    assert(0 == ret);

    printf("App ended.\n");



    return 1;
}

