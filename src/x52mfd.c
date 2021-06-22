/*
   libx52 related stuff and wrappers
   */

#include <stdio.h>
#include <unistd.h>

#include "x52mfd.h"



// device connection re-establisher
void x52mfd_can(x52mfd_t *x52mfd) {
    while (!libx52_is_connected(x52mfd->dev)) {
        if (libx52_connect(x52mfd->dev) == LIBX52_SUCCESS)
            break;
        else
            sleep(1);
    }
}

// create a thread that will keep joy connection alive
int x52mfd_init(x52mfd_t *x52mfd) {
    int rc;

    // we can't go on without this
    rc = libx52_init(&x52mfd->dev);
    if (rc != LIBX52_SUCCESS) {
        fprintf(stderr, "Failed to init libx52: %s\n", libx52_strerror(rc));
        return 1;
    }

    return 0;
}

// disconnect from joy
int x52mfd_disconnect(x52mfd_t *x52mfd) {
    libx52_disconnect(x52mfd->dev);
    libx52_exit(x52mfd->dev);
    return 0;
}


