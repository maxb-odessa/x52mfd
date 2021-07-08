
#include <stdio.h>
#include <unistd.h>

#include "x52mfd.h"

void * prg_writer(void *arg) {
    ctx_t *ctx = (ctx_t *)arg;

    while (!ctx->done)
        sleep(1);

    puts("wr done");

    return ctx;
}

