
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "x52mfd.h"

libx52_device *x52;

int x52mfd_mod_init(const char **err) {
    int rc;

    rc = libx52_init(&x52);
    if (rc != LIBX52_SUCCESS) {
        *err = libx52_strerror(rc);
        return 1;
    }

    rc = libx52_connect(x52);
    if (rc != LIBX52_SUCCESS) {
        *err = libx52_strerror(rc);
        return 1;
    }

    return 0;
}

int x52mfd_mod_run(const char **err) {
    char tbuf[16 + 1];
    time_t now;
    struct tm *tm;

    memset(tbuf, '-', sizeof(tbuf));
    libx52_set_text(x52, 2, tbuf, sizeof(tbuf));

    while (1) {
        now = time(NULL);
        tm = localtime(&now);

        strftime(tbuf, sizeof(tbuf) - 1, "%d %B %Y", tm);
        libx52_set_text(x52, 0, tbuf, strlen(tbuf));

        strftime(tbuf, sizeof(tbuf) - 1, "%H:%M:%S", tm);
        libx52_set_text(x52, 1, tbuf, strlen(tbuf));

        libx52_update(x52);

        sleep(1);
    }

    return 0;
}

int x52mfd_mod_finish(const char **err) {
    libx52_exit(x52);
    return 0;
}

