
#include <unistd.h>
#include <stdio.h>

#include "x52mfd.h"


// thread loop
// init x52 lib and keep its connection alive
void *joy_connector(void *arg) {
    ctx_t *ctx = (ctx_t *)arg;
    int rc;

    // init x52 lib
    pthread_mutex_lock(&ctx->mutex);

    rc = libx52_init(&ctx->x52dev);
    if (rc != LIBX52_SUCCESS) {
        plog("libx52 init failed: %s\n", libx52_strerror(rc));
        // we can't go on as well as other threads
        ctx->done = 1;
    } else {
        // this odd but libx52_is_connected() does not detect disconnection without this
        libx52_connect(ctx->x52dev);
        libx52_disconnect(ctx->x52dev);
    }

    pthread_mutex_unlock(&ctx->mutex);

    // keep joy connection alive
    while (! ctx->done) {

        // is dev connection is ok
        // this check will return valid value only after libx52_update() call
        // so if there were no such calls and a joy is disconnected then we won't know it
        if (! libx52_is_connected(ctx->x52dev)) {

            ctx->connected = 0;

            // need to reconnect
            pthread_mutex_lock(&ctx->mutex);
            rc = libx52_connect(ctx->x52dev);
            if (rc != LIBX52_SUCCESS) {
                //plog("connection to joystick failed: %s\n", libx52_strerror(rc));
                usleep(LOOP_DELAY_US * 10);
            } else {
                plog("connected to x52pro joystick\n");
                ctx->connected = 1;
                pthread_cond_signal(&ctx->connected_condvar);
            }
            pthread_mutex_unlock(&ctx->mutex);

        }

        // some delay between checks
        usleep(LOOP_DELAY_US);
    }

    // cleanups
    pthread_mutex_lock(&ctx->mutex);
    ctx->connected = 0;
    if (libx52_is_connected(ctx->x52dev))
        libx52_disconnect(ctx->x52dev);
    libx52_exit(ctx->x52dev);
    pthread_mutex_unlock(&ctx->mutex);

    // done
    return NULL;
}

