#include <internal/debug/debug.h>
#include <stdio.h>
#include <stdarg.h>


void debug(const char *format, ...) {
#ifdef DEBUG_OUTPUT
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
#endif
}