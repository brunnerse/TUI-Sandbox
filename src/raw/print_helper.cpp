#include "print_helper.h"

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

int print_repeated(const char *str, int num_repetitions) {
    int ret;
    for (unsigned i = 0; i < num_repetitions; i++)
        ret += printf("%s", str);
    return ret;
}

int printf_aligned(unsigned width, Align alignment, const char* fmt, ...) {
    int length;
    va_list arg;
    va_start(arg, fmt);

    char str[200];
    length = vsnprintf(str, 200, fmt, arg);
    va_end(arg);

    // Cut off string if too long
    if (length > width) {
        width = length;
        str[width] = '\0';
    }

    uint16_t n_before, n_after;

    if (alignment == Align::LEFT) {
        n_before = 0;
        n_after = width - length;
    } else if (alignment == Align::CENTER) {
        n_before = (width - length) / 2;  // Round down
        n_after = (width - length + 1) / 2; // Round up
    } else if (alignment == Align::RIGHT) {
        n_before = width - length;
        n_after = 0; 
    } else 
            assert(0);
//    fprintf(stderr, "n_before: %u, n_after: %u\n", n_before, n_after);

    printf("%*s%s%*s", n_before, "", str, n_after, "");

    return length;
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
