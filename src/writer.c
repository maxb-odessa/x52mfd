
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>

#include "x52mfd.h"

static int send_changes(ctx_t *ctx, libx52io_report *curr, libx52io_report*prev);

// write data (message) to proggie stdin
// return 0 if ok
int prg_writer_aux(ctx_t *ctx, char *data) {
    static pthread_mutex_t wmutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&wmutex);

    fd_set outfds;
    struct timeval tv = {0, LOOP_DELAY_US};
    int rc;

    // can we write?
    FD_ZERO(&outfds);
    FD_SET(ctx->outfd, &outfds);

    rc = select(ctx->outfd + 1, NULL, &outfds, NULL, &tv);
    if (rc < 0) {
        perror("select()");
        goto done;
    }

    // can write - do it
    if (rc > 0) {
        rc = write(ctx->outfd, data, strlen(data));
        if (rc <= 0)
            plog("writer: write to proggie failed: %s\n", strerror(errno));
    }

done:
    pthread_mutex_unlock(&wmutex);

    return rc > 0;
}


// read joy events and send them to proggie
void * prg_writer(void *arg) {
    ctx_t *ctx = (ctx_t *)arg;
    int disco_sent = 0, conn_sent = 0;
    libx52io_report prev_report = {{0}, {0}, 0, 0};
    libx52io_report curr_report = {{0}, {0}, 0, 0};

    while (! ctx->done) {

        // inform a proggie on our joy connection status
        if (! ctx->x52dev_ok || ! ctx->x52ctx_ok) {
            if (! disco_sent) {
                prg_writer_aux(ctx, "DISCONNECTED\n");
                disco_sent = 1;
                conn_sent = 0;
            }
            usleep(LOOP_DELAY_US);
            continue;
        } else {
            if (! conn_sent) {
                prg_writer_aux(ctx, "CONNECTED\n");
                conn_sent = 1;
                disco_sent = 0;
            }
        }

        // wait for joystick events
        if (libx52io_read_timeout(ctx->x52ctx, &curr_report, LOOP_DELAY_US) != LIBX52IO_SUCCESS)
            continue;

        // see what changed (buttons and such) and report them to proggie
        send_changes(ctx, &curr_report, &prev_report);

        // save prev joy state
        prev_report = curr_report;

    }

    // we're done
    //plog("writer: exited\n");
    return NULL;
}


// HAT positions
static char *hatpos[] = { "", "N", "NE", "E", "SE", "S", "SW", "W", "NW" };

// see the diff between cur and prev joy states and send changes to proggie
int send_changes(ctx_t *ctx, libx52io_report *curr, libx52io_report *prev) {
    char buf[128];  // button names are not long

    // compare axis (NOT SUPPORTED ATM, I see no use of this)

    // compare buttons
    for (int i = LIBX52IO_BUTTON_MAX - 1; i >= 0; i --) {
        if (curr->button[i] != prev->button[i]) {
            strncpy(buf, libx52io_button_to_str(i), sizeof(buf) - 16);
            if (curr->button[i] == 0)
                strcat(buf, " off\n");
            else
                strcat(buf, " on\n");
            prg_writer_aux(ctx, buf);
        }
    }

    // compare mode states
    if (curr->mode != prev->mode) {
        snprintf(buf, sizeof(buf) - 1, "MODE %d\n", curr->mode);
        prg_writer_aux(ctx, buf);
    }

    // compare hat states (also emulate POV behavior)
    if (curr->hat != prev->hat) {
        if (prev->hat > 0) {
            strncpy(buf, "BTN_HAT_", sizeof(buf) - 1);
            strncat(buf, hatpos[prev->hat], sizeof(buf) - 1);
            strncat(buf, " off\n", sizeof(buf) - 1);
            prg_writer_aux(ctx, buf);
        }
        if (curr->hat > 0) {
            strncpy(buf, "BTN_HAT_", sizeof(buf) - 1);
            strncat(buf, hatpos[curr->hat], sizeof(buf) - 1);
            strncat(buf, " on\n", sizeof(buf) - 1);
            prg_writer_aux(ctx, buf);
        }
    }

    // done
    return 0;
}

