
#include <stdio.h>

#include "ed-odyssey.h"

libx52_device *x52dev;


int mod_ed_setup(const char **err) {

    int rc;

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

int mod_ed_loop(const char **err) {

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

int mod_ed_done(const char **err) {


    // close file

    // restore joy state

    return 0;
}

// expose this module to the caller
X52MFD_INIT(mod_ed_setup);
X52MFD_RUN(mod_ed_loop);
X52MFD_FINISH(mod_ed_done);


