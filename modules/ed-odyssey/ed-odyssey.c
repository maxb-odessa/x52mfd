
#include <stdio.h>

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

    // search for commander's journal and try to open it
    // TODO
    return 0;
}

static int mod_ed_loop(x52mfd_t *x52mfd) {

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


