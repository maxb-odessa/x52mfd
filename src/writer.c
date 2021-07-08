
#include <stdio.h>
#include <unistd.h>

#include "x52mfd.h"

void * prg_writer(void *arg) {
    ctx_t *ctx = (ctx_t *)arg;

    while (!ctx->done)
        usleep(LOOP_DELAY_US);

    puts("wr done");

    return ctx;
}

