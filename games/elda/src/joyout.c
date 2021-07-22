
#include "elda.h"

#include <sys/select.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


// init joyout events
bool joyout_events_init(void) {
    // nothing to do here atm
    return true;
}


#define BUFSIZE    1024
static char buf[BUFSIZE + 1];
static size_t buflen;


// wait (with timeout) for events from stdin and return them
bool joyout_event_get(char **bufp) {
    fd_set rfds;
    struct timeval tv = { .tv_sec = 0, .tv_usec = EVENT_TIMEOUT_US };
    int rc;
    char *nlp;

    // here we have either empty buffer or incomplete line in it (i.e. not ending with \n)

    // shift the buf
    // do this before any fd reading to fetch all the lines
    // that are already accumulated in the buffer
    if (buflen > 0) {
        memmove(buf, buf + buflen + 1, BUFSIZE - buflen);
        buflen = strlen(buf);
    }

    // setup select() for stdin
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    // wait for data
    rc = select(1, &rfds, NULL, NULL, &tv);

    // no data on stdin, it's ok
    if (rc == 0)
        return true;
    else if (rc < 0) {
        // select() failed, this is bad
        plog("select(stdin) failed: %s\n", strerror(errno));
        return false;
    }

    // fetch next data portion from fd - append it to what is already present in buf
    rc = read(0, buf + buflen, BUFSIZE - buflen);
    if (rc < 0) {
        plog("read(stdin) failed: %s\n", strerror(errno));
        return false;
    }

    // it's ok if read() returned no data - we'll examine our buffer

    // search for next '\n'
    nlp = strchr(buf, '\n');

    // no newline yet - continue accumulating line in buf
    if (! nlp)
        *bufp = NULL;
    // got newline - cut the buf on it
    else {
        *nlp ++ = '\0';
        buflen = nlp - buf;
        *bufp = buf;
    }

    // we're done
    return true;
}


