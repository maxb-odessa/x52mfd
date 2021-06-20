
#include <stdio.h>

#include "ed-odyssey.h"

static ed_action_t *actions;

#define ED_ACTIONS_FILE_VAR "ED_ODYSSEY_ACTIONS_FILE"
#define ED_JOURNAL_DIR_VAR  "ED_JOURNAL_DIR" 

static int mod_ed_setup(x52mfd_t *x52mfd) {
    char *actions_file = getenv(ED_ACTIONS_FILE_VAR);
    char *journal_dir = getenv(ED_JOURNAL_DIR_VAR);


    // read and parse actions file (take its location from env var)
    if (! actions_file) {
        x52mfd->err = "Env var '" ED_ACTIONS_FILE_VAR "' not set";
        return 1;
    }

    if (parse_actions_file(actions_file, &actions, &x52mfd->err))
        return 1;

    // search for commander's journal and try to open it


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

    // restore joy state

    return 0;
}

// expose this module to the caller
X52MFD_INIT(mod_ed_setup);
X52MFD_RUN(mod_ed_loop);
X52MFD_FINISH(mod_ed_done);


