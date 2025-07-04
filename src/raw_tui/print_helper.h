#pragma once

#include <stdio.h>
#include <stdint.h>


enum class Align {
	LEFT,
	CENTER,
	RIGHT
};

int printf_aligned(unsigned width, Align alignment, const char* fmt, ...) __attribute__((format (printf, 3, 4)));
int printf_aligned(unsigned width, Align alignment, char align_char, const char* fmt, ...) __attribute__((format (printf, 4, 5)));