
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "x52mfd.h"

int mod_st_setup(x52mfd_t *x52mfd) {
    return 0;
}

int mod_st_loop(x52mfd_t *x52mfd) {
    char tbuf[16 + 1];
    time_t now;
    struct tm *tm;

    memset(tbuf, '-', sizeof(tbuf));
    x52mfd_can(x52mfd);
    libx52_set_text(x52mfd->dev, 2, tbuf, sizeof(tbuf));

    while (1) {
        now = time(NULL);
        tm = localtime(&now);

        x52mfd_can(x52mfd);

        strftime(tbuf, sizeof(tbuf) - 1, "%d %B %Y", tm);
        libx52_set_text(x52mfd->dev, 0, tbuf, strlen(tbuf));

        strftime(tbuf, sizeof(tbuf) - 1, "%H:%M:%S", tm);
        libx52_set_text(x52mfd->dev, 1, tbuf, strlen(tbuf));

        libx52_set_clock(x52mfd->dev, now, 0);

        libx52_update(x52mfd->dev);

        sleep(1);
    }

    return 0;
}

int mod_st_done(const char **err) {
    return 0;
}

// expose this module to the caller
X52MFD_INIT(mod_st_setup);
X52MFD_RUN(mod_st_loop);
X52MFD_FINISH(mod_st_done);


