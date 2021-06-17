
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "x52mfd.h"

libx52_device *x52dev;

int mod_st_setup(const char **err) {
    int rc;

    rc = libx52_init(&x52dev);
    if (rc != LIBX52_SUCCESS) {
        *err = libx52_strerror(rc);
        return 1;
    }

    rc = libx52_connect(x52dev);
    if (rc != LIBX52_SUCCESS) {
        *err = libx52_strerror(rc);
        return 1;
    }

    return 0;
}

int mod_st_loop(const char **err) {
    char tbuf[16 + 1];
    time_t now;
    struct tm *tm;

    memset(tbuf, '-', sizeof(tbuf));
    libx52_set_text(x52dev, 2, tbuf, sizeof(tbuf));

    while (1) {
        now = time(NULL);
        tm = localtime(&now);

        strftime(tbuf, sizeof(tbuf) - 1, "%d %B %Y", tm);
        libx52_set_text(x52dev, 0, tbuf, strlen(tbuf));

        strftime(tbuf, sizeof(tbuf) - 1, "%H:%M:%S", tm);
        libx52_set_text(x52dev, 1, tbuf, strlen(tbuf));

        libx52_set_clock(x52dev, now, 0);

        libx52_update(x52dev);

        sleep(1);
    }

    return 0;
}

int mod_st_done(const char **err) {
    libx52_disconnect(x52dev);
    libx52_exit(x52dev);
    return 0;
}

// expose this module to the caller
X52MFD_INIT(mod_st_setup);
X52MFD_RUN(mod_st_loop);
X52MFD_FINISH(mod_st_done);


