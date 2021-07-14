/*
   a Primitive LOGger - log messages to stderr
   */

#include "elda.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif


// print log message to stderr
void plog(char *fmt, ...) {
    va_list ap;
    time_t now = time(NULL);
    char format[26 + 1 + strlen(fmt)];

    va_start(ap, fmt);

    // get current time
    ctime_r(&now, format);
    format[24] = '|';
    format[25] = '\0';

    // make new format
    strcat(format, fmt); 


    vfprintf(stderr, format, ap);
    va_end(ap);

    return;
}
