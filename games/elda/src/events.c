
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
#include <ctype.h>


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
static int open_journal(char *journal_file) {
    int fd;

    // open journal file
    fd = open(journal_file, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
        plog("failed to open journal file '%s': %s\n", journal_file, strerror(errno));
    else
        // seel journal file to its end
        lseek(fd, 0, SEEK_END);

    // good, return the fd
    return fd;
}


// find event and execute it
static bool process_event(EVENT_TYPE evtype, char *buf) {
    event_t *evp;
    char **subs;
    int subs_num = 0;
    bool ok = true;


    // match buf over events
    evp = conf_match_event(evtype, buf, &subs, &subs_num);
    if (! evp)
        return ok;

    // event matched - execute its actions
    ok = exec_actions(evp->actions, buf, subs, subs_num);

    // cleanups
    if (subs_num > 0)
        pcre_free_substring_list((const char **)subs);

    // done
    return ok;
}


#define FD_JOYOUT_ID    0
#define FD_JOURNAL_ID   1

#define BUFSIZE    65535


// main events listening loop
// read events from journal and stdin and execute associated actions
bool events_loop(void) {
    char *journal_file;
    char buf[BUFSIZE + 1] = {0,}; // should be large enuff to hold whole event string from stdin or journal file
    size_t buflen = 0;
    bool go_on = true;
    struct pollfd pfds[2]; // 0 for stdin (joyout), 1 for journal
    char *nlp;
    int rc;

    // open and setup journal for poll()
    journal_file = find_journal_file();
    if (! journal_file) {
        plog("failed to read journal file\n");
        return false;
    }

    pfds[FD_JOURNAL_ID].fd = open_journal(journal_file);
    if (pfds[FD_JOURNAL_ID].fd < 0) {
        plog("failed to open journal\n");
        return false;
    }
    pfds[FD_JOURNAL_ID].events = POLLIN;
    plog("journal: watching '%s'\n", journal_file);

    // setup stdin for poll()
    pfds[FD_JOYOUT_ID].fd = fileno(stdin);
    pfds[FD_JOYOUT_ID].events = POLLIN;
    plog("joyout: watching 'stdin'\n");

    // poll for events
    while (go_on) {

        // wait for events
        // NOTE:
        // poll() on regular files always returns ready descriptor, so
        // all the code below is not effective
        if (poll(pfds, 2, -1) < 0) {
            plog("poll() failed: %s\n", strerror(errno));
            return false;
        }

        // let's sleep for a while to reduce cpu usage cased by poll() above
        usleep(30 * 1000); // 30ms


        // got events - process them
        for (int i = 0; i < 2; i ++) {

            // no event on this fd - skip it
            if (pfds[i].revents == 0 || !(pfds[i].revents & POLLIN))
                continue;

            // a CRUTCH:
            // in case of journal fd we'll try to read 1 char of data from it.
            // if we could - rewind fd position 1 char back for next read() call to get all the data
            if (i == FD_JOURNAL_ID) {
                char tbuf[1];
                rc = read(pfds[i].fd, tbuf, 1);
                if (rc == 1)
                    lseek(pfds[i].fd, -1, SEEK_CUR);
                else
                    continue;
            }

            // fetch next data portion from fd - append it to what is already present in buf
            rc = read(pfds[i].fd, buf + buflen, BUFSIZE - buflen - 1);
            // no data read
            if (rc <= 0) 
                continue;

            // draw the buf line-by-line
            while (1) {

                // shift the buf
                // do this before any fd reading to fetch all the lines
                // that are already accumulated in the buffer
                if (buflen > 0) {
                    memmove(buf, buf + buflen, BUFSIZE - buflen);
                    buflen = strlen(buf);
                }

                // search for next '\n'
                nlp = strchr(buf, '\n');
                // no newline yet - continue accumulating line in buf
                if (! nlp)
                    break;
                // got newline - cut the buf on it
                else
                    *nlp ++ = '\0';

                buflen = nlp - buf;
                plog("NLP2: <%s>\n", buf);

                // find matching event and execute it
                if (i == FD_JOYOUT_ID)
                    go_on = process_event(JOYOUT_EVENT, buf);
                else if (i == FD_JOURNAL_ID)
                    go_on = process_event(JOURNAL_EVENT, buf);

            }


        } // for(fds...)

    } // while(go_on...)

    // we're done
    return go_on;
}


