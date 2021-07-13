
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "ed-odyssey.h"

#define ED_PATTERNS_FILE_VAR "ED_ODYSSEY_PATTERNS_FILE"
#define ED_JOURNAL_DIR_VAR   "ED_JOURNAL_DIR"

#ifndef _ED_ODYSSEY_DEF_PATTERNS_PATH
#define _ED_ODYSSEY_DEF_PATTERNS_PATH "./ed-odyssey.json"
#endif

static ed_pattern_t *patterns, *begin, *end;

static int mod_ed_setup(x52mfd_t *x52mfd) {
    char *patterns_file = getenv(ED_PATTERNS_FILE_VAR);
    char *journal_dir = getenv(ED_JOURNAL_DIR_VAR);
    ed_pattern_t *b;


    // check for required paths
    if (! patterns_file) {
        // use default file location
        patterns_file = _ED_ODYSSEY_DEF_PATTERNS_PATH;
    }

    if (! journal_dir) {
        fprintf(stderr, "Env var '" ED_JOURNAL_DIR_VAR "' not set\n");
        return 1;
    }

    // read and parse actions file
    if (parse_patterns_file(patterns_file, &patterns, &begin, &end))
        return 1;

    // prepare journal processor
    if (journal_init(journal_dir))
        return 1;

    // prepare clock(s)
    libx52_set_clock_format(x52mfd->dev, LIBX52_CLOCK_1, LIBX52_CLOCK_FORMAT_24HR);
    libx52_set_date_format(x52mfd->dev, LIBX52_DATE_FORMAT_DDMMYY);

    // apply begin defaults
    b = begin;
    while (b) {
        x52mfd_can(x52mfd);
        pattern_apply_actions(x52mfd, b, "");
        b = b->next;
    }

    return 0;
}

static int mod_ed_loop(x52mfd_t *x52mfd) {
    char *journal_event;
    ed_pattern_t *ed_pattern;
    time_t prev_time = 0;

    while (! x52mfd_must_stop) {
        time_t now = time(NULL);

        // update date-time each 20sec
        if (now - prev_time >= 20) {
            x52mfd_can(x52mfd);
            libx52_set_clock(x52mfd->dev, now, 1);
            ed_led_apply(x52mfd->dev);
            prev_time = now;
        }

        // find event by pattern and apply it if found
        while ((journal_event = journal_get_event())) {
            ed_pattern = pattern_match_event(journal_event, patterns);
            if (ed_pattern) {
                x52mfd_can(x52mfd);
                pattern_apply_actions(x52mfd, ed_pattern, journal_event);
            }
        }

        // sleep 300 msec
        usleep(300*1000);

    }

    return 0;
}

static int mod_ed_done(x52mfd_t *x52mfd) {
    ed_pattern_t *e = end;

    // apply end
    e = end;
    while (e) {
        x52mfd_can(x52mfd);
        pattern_apply_actions(x52mfd, e, "");
        e = e->next;
    }

    return 0;
}

// expose this module to the caller
X52MFD_INIT(mod_ed_setup);
X52MFD_RUN(mod_ed_loop);
X52MFD_FINISH(mod_ed_done);


