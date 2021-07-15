
#include "elda.h"

#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fnmatch.h>
#include <dirent.h>
#include <assert.h>


// find freshest journal file
static char *find_journal_file() {
    struct dirent **journals;
    char *journal_dir, *journal_file;
    int num;
    char *journal_pattern;

    // preps
    if (! (journal_dir = conf_find_var("journal_dir"))) {
        plog("variable 'journal_dir' is not configured\n");
        return NULL;
    }

    if (! (journal_pattern = conf_find_var("journal_pattern"))) {
        plog("variable 'journal_pattern' is not configured\n");
        return NULL;
    }

    // CB for scandir(): filter out only journal files
    int filter_journal_name(const struct dirent *de) {
        return !fnmatch(journal_pattern, de->d_name, FNM_PATHNAME);
    }

    // find freshest journal file
    num = scandir(journal_dir, &journals, filter_journal_name, alphasort);
    if (num < 0) {
        plog("failed to scan journal dir '%s': %s\n", journal_dir, strerror(errno));
        return NULL;
    } else if (num == 0) {
        plog("matching journal file not found in '%s'\n", journal_dir);
        return NULL;
    }

    // found fresh journal - make full path to it
    journal_file = malloc(strlen(journal_dir) + strlen(journals[num - 1]->d_name) + 2);
    assert(journal_file);
    strcpy(journal_file, journal_dir);
    strcat(journal_file, "/");
    strcat(journal_file, journals[num - 1]->d_name);

    // done
    return journal_file;
}

// open journal file and return its fd
static int get_journal_fd(char *journal_file) {
    int jfd;

    // open journal file
    jfd = open(journal_file, O_RDONLY);
    if (jfd < 0) {
        plog("failed to open journal file '%s': %s\n", journal_file, strerror(errno));
        return -1;
    }

    // seel journal file to its end
    lseek(jfd, 0, SEEK_END);

    // good, return the fd
    return -1;
}


#define FD_JOYOUT  0
#define FD_JOURNAL 1

// main events listening loop
// read events from journal and stdin and execute associated actions
bool events_loop(void) {
    char *journal_file;
    char buf[65536] = {0,}; // should be large enuff to hold whole event string from stdin or journal file
    event_t *evp;
    char *subs[10]; // there can be not more than 10 subs, from $0 to $9
    bool go_on = true;
    struct pollfd pfds[2]; // 0 for stdin (joyout), 1 for journal
    int nfds = 1;

    // prepare epoll() struct
    // stdin is always available but not journal
    // so we'll wait for a journal to appear in the main loop
    pfds[FD_JOYOUT].fd = fileno(stdin);
    pfds[FD_JOYOUT].events = POLLIN;
    pfds[FD_JOURNAL].fd = -1;
    pfds[FD_JOURNAL].events = POLLIN;

    // init journal reading
    journal_file = find_journal_file();
    if (! journal_file) {
        plog("failed to read journal file\n");
        return false;
    }

    plog("journal - watching '%s'\n", journal_file);
    plog("joyout  - watching 'stdin'\n");

    // poll for events
    while (go_on) {

        // get journal fd if we can
        if (pfds[FD_JOURNAL].fd < 0)
            pfds[FD_JOURNAL].fd = get_journal_fd(journal_file);
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
            if (i == FD_JOYOUT)
                evp = conf_match_event(JOYOUT_EVENT, buf, subs);
            else if (i == FD_JOURNAL)
                evp = conf_match_event(JOURNAL_EVENT, buf, subs);

            // not matching event found
            if (! evp)
                continue;

            // event matched - process it
            go_on = exec_actions(evp->actions, buf, subs);

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


