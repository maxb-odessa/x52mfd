
#include "elda.h"

#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// open journal file and return its fd
int get_journal_fd(void) {

    return -1;
}


#define FD_STDIN   0
#define FD_JOURNAL 1

// main events listening loop
// read events from journal and stdin and execute associated actions
bool events_loop(void) {
    char buf[65536] = {0,}; // should be large enuff to hold whole event string from stdin or journal file
    event_t *evp;
    char *subs[10]; // there can be not more than 10 subs, from $0 to $9
    bool must_exit = false;
    struct pollfd pfds[2]; // 0 for stdin (joyout), 1 for journal
    int nfds = 1;

    // prepare epoll() struct
    // stdin is always available but not journal
    // so we'll wait for a journal to appear in the main loop
    pfds[FD_STDIN].fd = fileno(stdin);
    pfds[FD_STDIN].events = POLLIN;
    pfds[FD_JOURNAL].fd = -1;
    pfds[FD_JOURNAL].events = POLLIN;

    // poll for events
    while (! must_exit) {

        // get journal fd if we can
        pfds[FD_JOURNAL].fd = get_journal_fd();
        if (pfds[FD_JOURNAL].fd > 0)
            nfds = 2;
        else
            nfds = 1;

        // wait for events
        if (poll(pfds, nfds, -1) < 0) {
            plog("poll() failed: %s\n", strerror(errno));
            return false;
        }

        // got events - process them
        for (int i = 0; i < 2; i ++) {

            // no event on this fd - skip it
            if (pfds[i].revents == 0 || !(pfds[i].revents & POLLIN))
                continue;

            // read data from fd
            if (read(pfds[i].fd, buf, sizeof(buf) - 1) <= 0) {
                close(pfds[i].fd);
                pfds[i].fd = -pfds[i].fd;
                continue;
            }

            // find matching event
            evp = NULL;
            if (i == FD_STDIN)
                evp = conf_match_event(JOYOUT_EVENT, buf, subs);
            else if (i == FD_JOURNAL)
                evp = conf_match_event(JOURNAL_EVENT, buf, subs);

            // not matching event found
            if (! evp)
                continue;

            // event matched - process it
            must_exit = exec_actions(evp->actions, buf, subs);

            // cleanups
            for (int i = 0; i < sizeof(subs)/sizeof(subs[0]); i ++) {
                if (subs[i]) {
                    free(subs[i]);
                    subs[i] = NULL;
                }
            }
            buf[0] = '\0';

        } // for(fds...)

    } // while(!exit...)

    // we're done
    return false;
}


