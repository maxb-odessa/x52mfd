
#include "elda.h"

#include <limits.h>
#include <poll.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fnmatch.h>
#include <assert.h>

#define BUFSIZE  65535

struct journal_data {
    char    buf[BUFSIZE + 1];
    size_t  buflen;
    char    *journal_dir;
    char    *journal_file;
    char    *journal_pattern;
    int     journal_fd;
    int     inotify_fd;
};

static struct journal_data jdata;


// open journal file, close old one
static bool reopen_journal_file() {
    int fd;
    char *jpath;

    // check
    if (! jdata.journal_file)
        return false;

    jpath = malloc(strlen(jdata.journal_dir) + strlen(jdata.journal_file) + 2);
    assert(jpath);
    strcpy(jpath, jdata.journal_dir);
    strcat(jpath, "/");
    strcat(jpath, jdata.journal_file);

    // open the file
    fd = open(jpath, O_RDONLY|O_NONBLOCK);
    if (fd < 0)
        plog("failed to open journal '%s': %s\n", jpath, strerror(errno));
    //else
    //    lseek(fd, 0, SEEK_END);

    // close old fd
    if (jdata.journal_fd)
        close(jdata.journal_fd);

    jdata.journal_fd = fd;

    return true;
}


// init journal events reader
bool journal_events_init(void) {

    // preps
    if (! (jdata.journal_dir = conf_find_var("journal_dir"))) {
        plog("variable 'journal_dir' is not configured\n");
        return false;
    }

    if (! (jdata.journal_pattern = conf_find_var("journal_pattern"))) {
        plog("variable 'journal_pattern' is not configured\n");
        return false;
    }

    // init inotify to watch journal dir
    jdata.inotify_fd = inotify_init1(IN_NONBLOCK);
    if (jdata.inotify_fd < 0) {
        plog("inotify_init1() failed: %s\n", strerror(errno));
        return false;
    }

    // add watcvh to journal dir
    if (inotify_add_watch(jdata.inotify_fd, jdata.journal_dir, IN_MODIFY) < 0) {
        plog("inotify_add_watch() failed: %s\n", strerror(errno));
        return false;
    }

    plog("journal events: watching dir '%s'\n", jdata.journal_dir);

    // done init
    return true;
}


#define INOTIFY_EV_MAX    1024
#define INOTIFY_EV_SIZE  (sizeof(struct inotify_event))
#define INOTIFY_BUF_LEN  (INOTIFY_EV_MAX * (INOTIFY_EV_SIZE + PATH_MAX))

// wait for journal event and return it
bool journal_event_get(char **bufp) {
    char ievbuf[INOTIFY_BUF_LEN];
    ssize_t ievlen;
    struct pollfd pfds;
    int rc;
    const struct inotify_event *iev;
    char *nlp;

    // setup poll for inotify
    pfds.fd = jdata.inotify_fd;
    pfds.events = POLLIN;

    // wait for inotify events
    rc = poll(&pfds, 1, EVENT_TIMEOUT_MS);
    if (rc < 0) {
        plog("poll() failed: %s\n", strerror(errno));
        return false;
    } else if (rc == 0)
        // no event - ok
        return true;

    // no event - return
    if (! (pfds.revents & POLLIN))
        return true;


    // read inotify evens
    ievlen = read(pfds.fd, ievbuf, INOTIFY_BUF_LEN);
    if (ievlen < 0) {
        plog("read() failed: %s\n", strerror(errno));
        return false;
    }

    // examine inotify events: pick our current journal file
    for (char *ievptr = ievbuf; ievptr < ievbuf + ievlen; ievptr += INOTIFY_EV_SIZE + iev->len) {

        iev = (const struct inotify_event *)ievptr;

        // skip empty inotify event
        // skip directory and non-modify inotify events
        // skip non-journal files
        if (iev->len == 0 || (iev->mask & IN_ISDIR) || 
                !(iev->mask & IN_MODIFY) ||
                fnmatch(jdata.journal_pattern, iev->name, FNM_PATHNAME))
            continue;

        // see if journal file name changed
        if (! jdata.journal_file || strcmp(jdata.journal_file, iev->name)) {

            // save new journal file name
            if (jdata.journal_file)
                free(jdata.journal_file);

            jdata.journal_file = strdup(iev->name);

            plog("journal events: monitoring file '%s'\n", jdata.journal_file);

            // open new journal file
            if (! reopen_journal_file())
                return false;

        }

    } //for(ievents...)

    // shift the buf
    // do this before any fd reading to fetch all the lines
    // that are already accumulated in the buffer
    if (jdata.buflen > 0) {
        memmove(jdata.buf, jdata.buf + jdata.buflen, BUFSIZE - jdata.buflen);
        jdata.buflen = strlen(jdata.buf);
    }


    // poll for journal events
    pfds.fd = jdata.journal_fd;
    pfds.events = POLLIN;
    if (poll(&pfds, 1, -1) < 0) {
        plog("poll() failed: %s\n", strerror(errno));
        return false;
    }

    // read events from journal
    if (pfds.revents & POLLIN) {
        rc = read(pfds.fd, jdata.buf + jdata.buflen, BUFSIZE - jdata.buflen);
        if (rc < 0) {
            plog("read() failed: %s\n", strerror(errno));
            return false;
        }
    }

    // it's ok if read() returned no data - we'll examine our buffer

    // search for next '\n'
    nlp = strchr(jdata.buf, '\n');

    // no newline yet - continue accumulating line in buf
    if (! nlp)
        *bufp = NULL;
    else {
        // got newline - cut the buf on it
        *nlp ++ = '\0';
        jdata.buflen = nlp - jdata.buf;
        *bufp = jdata.buf;
    }

    // done
    return true;
}

