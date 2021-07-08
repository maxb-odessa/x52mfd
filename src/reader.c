
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>

#include "x52mfd.h"

static int read_cmd(ctx_t *ctx, char *buf, int *buflen);
static int parse_cmd(char *buf, char *cmd[]);
static int execute_cmd(ctx_t *ctx, char *cmd[]);

// let's expect cmd line from prog not longer than this
#define BUFSIZE 1024


// reader loop: read commands from prog fd and execute them
// not thread safe but it is in a single thread, right?
void *prg_reader(void *arg) {
    ctx_t *ctx = (ctx_t *)arg;
    char buf[BUFSIZE + 1] = {0,};
    char *cmd[1+4];  // the longest cmd is 'date' which has 4 args
    int rc;
    int buflen = 0;

    while (! ctx->done) {

        // read command and its arg(s) from prg
        rc = read_cmd(ctx, buf, &buflen);
        if (rc < 0) { // system failure
            ctx->done = 1;
            break;
        } else if (rc == 0)  // no data yet
            continue;

        // parse and execute command
        if (parse_cmd(buf, cmd) != 0)
            execute_cmd(ctx, cmd);

        // indicate we're done with this buffer
        // next call to read_cmd() will reset the buff
        *buf = '\0';
    }

    // we're done
    return NULL;
}



// read command from prog fd
// one command - one line, so we'll accumulate data until '\r' or EOF
int read_cmd(ctx_t *ctx, char *buf, int *buflen) {
    fd_set infds;
    struct timeval tv;
    char *nr;
    int rc;

    FD_ZERO(&infds);
    FD_SET(ctx->infd, &infds);

    tv.tv_sec = 0;
    tv.tv_usec = LOOP_DELAY_US;

    rc = select(ctx->infd + 1, &infds, NULL, NULL, &tv);

    // select failed?
    switch (rc) {
        case -1 :
            perror("select()");
        case 0 :
            return rc; // no data - it's ok
            break;
        default :
            break;
    }

    // nothing to read
    if (rc == 0)
        return 0;

    // the buffer was read an processed, need to shift it now
    if (*buflen > 0 && *buf == '\0') {
        memmove(buf, buf + (*buflen - 1), BUFSIZE - *buflen);
        *buflen = strlen(buf);
    }

    // read and append to buffer
    rc = read(ctx->infd, buf + *buflen, BUFSIZE - *buflen);

    // read failed?
    switch (rc) {
        case -1 :
            perror("read()");
        case 0:
            return -1;
            break;
        default:
            break;
    }

    // recalc buflen
    *buflen += rc;

    // buffer is full - return it
    if (*buflen == BUFSIZE)
        return 1;

    // got '\n' - indicate we've got whole string
    nr = strchr(buf, '\n');
    if (nr) {
        *nr = '\0';
        return 1;
    }

    // cmd string is incomplete yet
    return 0;
}


// parse command
// "this \"is\" a string" will be considered as one arg
int parse_cmd(char *buf, char *cmd[]) {
    fprintf(stderr, "READ: <%s>\n", buf);
    return 0;
}


// execute command
int execute_cmd(ctx_t *ctx, char *cmd[]) {

    return 0;
}
