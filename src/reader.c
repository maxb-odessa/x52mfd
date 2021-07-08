
#include <stdio.h>
#include <unistd.h>

#include "x52mfd.h"

void *prg_reader(void *arg) {
    ctx_t *ctx = (ctx_t *)arg;

    while (!ctx->done)
        sleep(1);

    puts("reader done");

    return ctx;
}
