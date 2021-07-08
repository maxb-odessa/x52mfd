
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "x52mfd.h"

static int read_cmd(ctx_t *ctx, char *buf);
static int parse_cmd(char *buf, char *cmd[]);
static int execute_cmd(ctx_t *ctx, char *cmd[]);

// let's expect cmd line from prog not longer than this
#define BUFSIZE 128


// reader loop: read commands from prog fd and execute them
// not thread safe but it is in a single thread, right?
void *prg_reader(void *arg) {
    ctx_t *ctx = (ctx_t *)arg;
    char *cmd[1+4];  // the longest cmd is 'date' which has 4 args
    static char buf[BUFSIZE + 1] = {0,};
    int rc;

    while (! ctx->done) {

        // read command and its arg(s) from prg
        rc = read_cmd(ctx, buf);
        if (rc < 0) { // system failure
            ctx->done = 1;
            break;
        } else if (rc == 0) // no data
            continue;

        // parse and execute command
        if (parse_cmd(buf, cmd) == 0)
            execute_cmd(ctx, cmd);

        // cleanups!
        for (int i = 0; i < 5; i ++)
            if (cmd[i]) {
                free(cmd[i]);
                cmd[i] = NULL;
            }

    }

    // we're done
    return NULL;
}



// read command from prog fd
// one command - one line, so we'll accumulate data until '\n' or EOF
int read_cmd(ctx_t *ctx, char *buf) {
    fd_set infds;
    struct timeval tv;
    char *nr;
    int rc;
    size_t buflen = strlen(buf); // calc buf part to first '\0'

    // shift the buf
    if (buflen > 0) {
        memmove(buf, buf + buflen + 1, BUFSIZE - buflen + 1);
        buflen = strlen(buf);
    }

    // is data available?
    FD_ZERO(&infds);
    FD_SET(ctx->infd, &infds);

    tv.tv_sec = 0;
    tv.tv_usec = LOOP_DELAY_US;

    rc = select(ctx->infd + 1, &infds, NULL, NULL, &tv);
    if (rc < 0) {
        perror("select()");
        return -1;
    } 

    // got some data - read it
    // read and append to buffer
    if (rc > 0) {
        rc = read(ctx->infd, buf + buflen, BUFSIZE - buflen);
        if (rc < 0) {
            perror("read()");
            return -1;
        }
    }

    // search for next '\n' and cut the buf there
    nr = strchr(buf, '\n');
    if (nr) {
        *nr = '\0';
        return 1; // got full cmd
    }

    // cmd string is incomplete yet
    return 0;
}




// parse command
// "this \"is\" a string" will be considered as one arg
int parse_cmd(char *buf, char *cmd[]) {
    char *p1, *p2;
    int cmd_idx = 0;

    p1 = p2 = buf;
    while (*p1) {

        // move p1 to first non-blank
        while (*p1 && isspace(*p1))
            p1 ++;

        // check p1
        if (! *p1)
            break;

        // move p2 to first non-blank after p1
        // but! if p1 points at \" then we must find another \"
        if (*p1 == '\"') {
            p2 = ++ p1; // p1 must skip current \"
            while (*p2 && *p2 != '\"')
                p2 ++;
        } else {
            p2 = p1 + 1; // p1 must stay where it is
            while (*p2 && !isspace(*p2))
                p2 ++;
        }

        // check p2 not needed - could be end-of-string

        // got a word!
        cmd[cmd_idx++] = strndup(p1, p2 - p1);

        // next word awaits
        if (*p2)
            p2 ++;
        p1 = p2;

        // enuff
        if (cmd_idx > 4)
            break;

    }


    // nothing parsed
    if (! cmd[0])
        return 1;

    for (int i = 0; i < 5; i ++)
        if (cmd[i])
            fprintf(stderr, "CMD[%d] = <%s>\n", i, cmd[i]);

    return 0;
}


// execute command
int execute_cmd(ctx_t *ctx, char *cmd[]) {

    return 0;
}


