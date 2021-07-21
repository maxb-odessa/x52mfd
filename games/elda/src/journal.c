
#include "elda.h"

#include <limits.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fnmatch.h>
#include <assert.h>

#define BUFSIZE  65535

struct journal_data {
    char buf[BUFSIZE + 1];
    char *journal_dir;
    char *journal_file;
    char *journal_pattern;
    FILE *journal_fp;
    int  inotify_fd;
};

static struct journal_data jdata;

#if 0
// open journal file, close old one
static bool reopen_journal_file1() {
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
    fd = open(jpath, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
        plog("failed to open journal '%s': %s\n", jpath, strerror(errno));
    else
        lseek(fd, 0, SEEK_END);

    // close old fd
    if (jdata.journal_fd)
        close(jdata.journal_fd);

    jdata.journal_fd = fd;

    return true;
}
#endif


// open journal file, close old one
static bool reopen_journal_file() {
    FILE *fp = NULL;
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
    fp = fopen(jpath, "r");
    if (! fp)
        plog("failed to open journal '%s': %s\n", jpath, strerror(errno));
    else
        fseek(fp, 0, SEEK_END);

    // close old fd
    if (jdata.journal_fp)
        fclose(jdata.journal_fp);

    jdata.journal_fp = fp;

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
    fd_set rfds;
    struct timeval tv = { .tv_sec = 0, .tv_usec = EVENT_TIMEOUT_US };
    int rc;
    const struct inotify_event *iev;

    // setup select(fd)
    FD_ZERO(&rfds);
    FD_SET(jdata.inotify_fd, &rfds);

    // wait for inotify events
    rc = select(jdata.inotify_fd + 1, &rfds, NULL, NULL, &tv);
    if (rc < 0) {
        // failure
        plog("select() failed: %s\n", strerror(errno));
        return false;
    } else if (rc == 0)
        // no event - ok
        return true;


    // read inotify events from fd
    ievlen = read(jdata.inotify_fd, ievbuf, INOTIFY_BUF_LEN);
    if (ievlen < 0) {
        plog("read() failed: %s\n", strerror(errno));
        return false;
    }

    // examine inotify events
    for (char *ievptr = ievbuf; ievptr < ievbuf + ievlen; ievptr += INOTIFY_EV_SIZE + iev->len) {

        iev = (const struct inotify_event *)ievptr;

        // skip empty inotify event
        if (iev->len == 0)
            continue;

        // skip directory and non-modify inotify events
        if ((iev->mask & IN_ISDIR) || ! (iev->mask & IN_MODIFY))
            continue;

        // ignore non-journal files
        if (fnmatch(jdata.journal_pattern, iev->name, FNM_PATHNAME) != 0)
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

        tv.tv_sec = 0;
        tv.tv_usec = EVENT_TIMEOUT_US;

        // setup select(fd)
        FD_ZERO(&rfds);
        FD_SET(fileno(jdata.journal_fp), &rfds);

        // wait for inotify events
        rc = select(fileno(jdata.journal_fp) + 1, &rfds, NULL, NULL, &tv);
        if (rc < 0) {
            // failure
            plog("select() failed: %s\n", strerror(errno));
            return false;
        } else if (rc == 0)
            // no event - ok
            return true;


        // read line from journal file
        if (fgets(jdata.buf, BUFSIZE, jdata.journal_fp) != NULL)
            *bufp = jdata.buf;
        plog("B: r:%d l:%d <%s>\n", rc, strlen(jdata.buf), jdata.buf);

        // ignore the rest of inotify events
        break;

    } //for(ievents...)

    // done
    return true;
}

