
#include <unistd.h>
#include <stdio.h>

#include "x52mfd.h"

void *joy_connector(void *arg) {
    ctx_t *ctx = (ctx_t  *)arg;

    while (! ctx->done)
        sleep(1);
    fputs("conn done", stderr);

    return ctx;
}

