
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "x52mfd.h"

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif


// print log message to stderr
void plog(char *fmt, ...) {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mutex);

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

    pthread_mutex_unlock(&mutex);
    return;
}
