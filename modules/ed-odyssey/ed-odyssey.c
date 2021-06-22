
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "ed-odyssey.h"

#define ED_PATTERNS_FILE_VAR "ED_ODYSSEY_PATTERNS_FILE"
#define ED_JOURNAL_DIR_VAR   "ED_JOURNAL_DIR" 

static ed_pattern_t *patterns;

static int mod_ed_setup(x52mfd_t *x52mfd) {
    char *patterns_file = getenv(ED_PATTERNS_FILE_VAR);
    char *journal_dir = getenv(ED_JOURNAL_DIR_VAR);


    // check for required paths
    if (! patterns_file) {
        fprintf(stderr,"Env var '" ED_PATTERNS_FILE_VAR "' not set\n");
        return 1;
    }

    if (! journal_dir) {
        fprintf(stderr, "Env var '" ED_JOURNAL_DIR_VAR "' not set\n");
        return 1;
    }

    // read and parse actions file
    if (parse_patterns_file(patterns_file, &patterns))
        return 1;

    // prepare journal processor
    if (journal_init(journal_dir))
        return 1;

    return 0;
}

static int mod_ed_loop(x52mfd_t *x52mfd) {
    char *journal_event;
    ed_pattern_t *ed_pattern;

    // prepare clock(s)
    libx52_set_clock_format(x52mfd->dev, LIBX52_CLOCK_1, LIBX52_CLOCK_FORMAT_24HR);
    libx52_set_date_format(x52mfd->dev, LIBX52_DATE_FORMAT_DDMMYY);

    while (1) {

        // sleep 1 sec
        sleep(1);

        // update date-time
        libx52_set_clock(x52mfd->dev, time(NULL), 1);
        ed_led_apply(x52mfd->dev);

        journal_event = journal_get_event();
        if (! journal_event)
            continue;

        // find event by pattern
        ed_pattern = pattern_match_event(journal_event, patterns);
        if (! ed_pattern)
            continue;

        // apply actions (parse event for MFD* if needed)

    }


#if 0
    ed_led_action_t *action;

    x52mfd_can(x52mfd);

    action = ed_led_parse_action("MFD", "100%");
    printf("%d\n", ed_led_set(x52mfd->dev, action, NULL));

    action = ed_led_parse_action("LEDS", "blink");
    printf("%d\n", ed_led_set(x52mfd->dev, action, NULL));

    action = ed_led_parse_action("MFD1", "some undefined");
    printf("%d\n", ed_led_set(x52mfd->dev, action, "AAAAAAAAAAA"));

    action = ed_led_parse_action("A", "green");
    printf("%d\n", ed_led_set(x52mfd->dev, action, NULL));


    // find file

    // open file

    // watch file changes

    // read line

    // parse line

    // search for action

    // do action

    ed_led_apply(x52mfd->dev);
#endif
    return 0;
}

static int mod_ed_done(x52mfd_t *x52mfd) {


    // close file

    // restore joy state(?)

    return 0;
}

// expose this module to the caller
X52MFD_INIT(mod_ed_setup);
X52MFD_RUN(mod_ed_loop);
X52MFD_FINISH(mod_ed_done);


