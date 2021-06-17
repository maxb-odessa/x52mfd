
#include <stdio.h>

#include "ed-odyssey.h"

static libx52_device *x52dev;
static ed_action_t *actions;

#define ED_ACTIONS_FILE_VAR "ED_ODYSSEY_ACTIONS_FILE"
#define ED_JOURNAL_DIR_VAR  "ED_JOURNAL_DIR" 

static int mod_ed_setup(const char **err) {
    int rc;
    char *actions_file = getenv(ED_ACTIONS_FILE_VAR);
    char *journal_dir = getenv(ED_JOURNAL_DIR_VAR);


    // read and parse actions file (take its location from env var)
    if (! actions_file) {
        *err = "Env var '" ED_ACTIONS_FILE_VAR "' not set";
        return 1;
    }

    if (parse_actions_file(actions_file, &actions, err))
        return 1;

    // search for commander's journal and try to open it


    // init x52 and connect to it
    rc = libx52_init(&x52dev);
    if (rc != LIBX52_SUCCESS) {
        *err = libx52_strerror(rc);
        return 1;
    }

    rc = libx52_connect(x52dev);
    if (rc != LIBX52_SUCCESS) {
        *err = libx52_strerror(rc);
        return 1;
    }

    return 0;
}

static int mod_ed_loop(const char **err) {

    ed_led_action_t *action;

    action = ed_led_parse_action("MFD", "100%");
    printf("%d\n", ed_led_set(x52dev, action, NULL));

    action = ed_led_parse_action("LEDS", "blink");
    printf("%d\n", ed_led_set(x52dev, action, NULL));

    action = ed_led_parse_action("MFD1", "some undefined");
    printf("%d\n", ed_led_set(x52dev, action, "AAAAAAAAAAA"));

    action = ed_led_parse_action("A", "green");
    printf("%d\n", ed_led_set(x52dev, action, NULL));


    // find file

    // open file

    // watch file changes

    // read line

    // parse line

    // search for action

    // do action

    ed_led_apply(x52dev);

    return 0;
}

static int mod_ed_done(const char **err) {


    // close file

    // restore joy state

    return 0;
}

// expose this module to the caller
X52MFD_INIT(mod_ed_setup);
X52MFD_RUN(mod_ed_loop);
X52MFD_FINISH(mod_ed_done);


