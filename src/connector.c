
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

    // LEDS and MFD manipulator
    rc = libx52_init(&ctx->x52wr);
    if (rc != LIBX52_SUCCESS) {
        plog("connector: libx52 WRITE init failed: %s\n", libx52_strerror(rc));
        // we can't go on as well as other threads
        ctx->done = 1;
    } else {
        // this odd but libx52_is_connected() does not detect disconnection without this
        plog("connector: connecting to joystick WRITE dev\n");
        libx52_connect(ctx->x52wr);
        libx52_disconnect(ctx->x52wr);
    }

    // BUTTONS and AXIS (not all) reader
    rc = libx52io_init(&ctx->x52ro);
    if (rc != LIBX52IO_SUCCESS) {
        plog("connector: libx52io READ init failed: %s\n", libx52io_strerror(rc));
        // we can't go on as well as other threads
        ctx->done = 1;
    } else {
        // real connection will be done in the loop below
        plog("connector: connecting to joystick READ dev\n");
        libx52io_open(ctx->x52ro);
    }

    pthread_mutex_unlock(&ctx->mutex);

    // keep joy connection alive
    while (! ctx->done) {

        // is dev connection is ok
        // this check will return valid value only after libx52_update() call
        // so if there were no such calls and a joy is disconnected then we won't know it
        if (ctx->x52wr_ok && ! libx52_is_connected(ctx->x52wr)) {
            plog("connector: joystick WRITE dev is disconnected\n");
            ctx->x52wr_ok = 0;
        }

        // need to reconnect
        if (! ctx->x52wr_ok) {
            pthread_mutex_lock(&ctx->mutex);
            rc = libx52_connect(ctx->x52wr);
            if (rc != LIBX52_SUCCESS)
                usleep(LOOP_DELAY_US * 10);
            else {
                ctx->x52wr_ok = 1;
                plog("connector: connected to joystick WRITE dev\n");
            }
            pthread_mutex_unlock(&ctx->mutex);
        }

        // there is no such func for x52ctx like libx52_is_connected(), 
        // so we'll use libx52io_get_vendor_id() instead
        if (ctx->x52ro_ok && ! libx52io_get_vendor_id(ctx->x52ro)) {
            plog("connector: joystick READ dev is disconnected\n");
            ctx->x52ro_ok = 0;
        }

        // check and reconnect joy for reading
        if (! ctx->x52ro_ok) {
            pthread_mutex_lock(&ctx->mutex);
            rc = libx52io_open(ctx->x52ro);
            if (rc != LIBX52IO_SUCCESS)
                usleep(LOOP_DELAY_US * 10);
            else {
                ctx->x52ro_ok = 1;
                plog("connector: connected to %s %s, READ dev\n",
                        libx52io_get_manufacturer_string(ctx->x52ro),
                        libx52io_get_product_string(ctx->x52ro));

            }
            pthread_mutex_unlock(&ctx->mutex);
        }

        // some delay between checks
        usleep(LOOP_DELAY_US);
    }

    // cleanups
    ctx->x52wr_ok = 0;
    ctx->x52ro_ok = 0;

    pthread_mutex_lock(&ctx->mutex);

    libx52_disconnect(ctx->x52wr);
    libx52_exit(ctx->x52wr);

    libx52io_close(ctx->x52ro);
    libx52io_exit(ctx->x52ro);

    pthread_mutex_unlock(&ctx->mutex);


    // done
    //plog("connector: exited\n");
    return NULL;
}

