#include "print_helper.h"

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>



int printf_aligned(unsigned width, Align alignment, const char* fmt, ...) {
    va_list arg;
    va_start(arg, fmt);

    char str[200];
    int ret = vsnprintf(str, 200, fmt, arg);
    if (ret < 0)
        return ret;
    va_end(arg);

    unsigned length = (unsigned)ret;
    // Cut off string if too long
    if (length > width) {
        width = length;
        str[width] = '\0';
    }

    int n_before, n_after;

    if (alignment == Align::LEFT) {
        n_before = 0;
        n_after = (int)(width - length);
    } else if (alignment == Align::CENTER) {
        n_before = (int)(width - length) / 2;  // Round down
        n_after = (int)(width - length + 1) / 2; // Round up
    } else if (alignment == Align::RIGHT) {
        n_before = (int)(width - length);
        n_after = 0; 
    } else 
            assert(0);

    assert(n_before >= 0 && n_after >= 0);
//    fprintf(stderr, "n_before: %u, n_after: %u\n", n_before, n_after);

    return printf("%*s%s%*s", n_before, "", str, n_after, "");
}

/*
int printf_aligned(unsigned min_width, Align alignment, char align_char, const char* fmt, ...) {
    int length;
    char al_chr[2] = {align_char, '\0'};

    va_list arg;
    va_start(arg, fmt);

    if (alignment == Align::LEFT) {
        length = vprintf(fmt, arg);
        print_repeated(al_chr, min_width - length);
    } else {
        char str[200];
        length = vsnprintf(str, 200, fmt, arg);

        if (alignment == Align::CENTER) {
            unsigned num_al_chrs =  
        }
        else if (alignment == Align::RIGHT) {

        } else 
            assert(0);
    }

    va_end(arg);
    return length;
}
*/
