
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ed-odyssey.h"

static int mfd_text(libx52_device *x52dev, int lineno, ed_led_state_t *DUMMY, char *data) {
    return libx52_set_text(x52dev, lineno, data, strlen(data));
}

static int led_state(libx52_device *x52dev, int id, ed_led_state_t *state, char *DUMMY) {
    return state->callback(x52dev, id, state->state);
}

typedef int (*cb_t)();

static ed_led_type_t led_types[] = {
    // mfd lines
    {"MFD1",     ED_LED_TYPE_TEXT,  0,                      (cb_t)mfd_text},
    {"MFD2",     ED_LED_TYPE_TEXT,  1,                      (cb_t)mfd_text},
    {"MFD3",     ED_LED_TYPE_TEXT,  2,                      (cb_t)mfd_text},
    // all leds and mfd only
    {"MFD",      ED_LED_TYPE_MFD,   1,                      (cb_t)led_state},
    {"LEDS",     ED_LED_TYPE_LEDS,  0,                      (cb_t)led_state},
    // each led
    {"FIRE",     ED_LED_TYPE_LED,   LIBX52_LED_FIRE,        (cb_t)led_state},
    {"A",        ED_LED_TYPE_LED,   LIBX52_LED_A,           (cb_t)led_state},
    {"B",        ED_LED_TYPE_LED,   LIBX52_LED_B,           (cb_t)led_state},
    {"D",        ED_LED_TYPE_LED,   LIBX52_LED_D,           (cb_t)led_state},
    {"E",        ED_LED_TYPE_LED,   LIBX52_LED_E,           (cb_t)led_state},
    {"T1",       ED_LED_TYPE_LED,   LIBX52_LED_T1,          (cb_t)led_state},
    {"T2",       ED_LED_TYPE_LED,   LIBX52_LED_T2,          (cb_t)led_state},
    {"T3",       ED_LED_TYPE_LED,   LIBX52_LED_T3,          (cb_t)led_state},
    {"POV",      ED_LED_TYPE_LED,   LIBX52_LED_POV,         (cb_t)led_state},
    {"CLUTCH",   ED_LED_TYPE_LED,   LIBX52_LED_CLUTCH,      (cb_t)led_state},
    {"THROTTLE", ED_LED_TYPE_LED,   LIBX52_LED_THROTTLE,    (cb_t)led_state},
};


#define PRCNT(X, Y) ((X) / 100 * (Y))

#define ED_LED_TYPE_ALL (ED_LED_TYPE_LEDS | ED_LED_TYPE_MFD)
static ed_led_state_t led_states[] = {
    // for each led
    {"OFF",     ED_LED_TYPE_LED,   LIBX52_LED_STATE_OFF,   (cb_t)libx52_set_led_state},
    {"ON",      ED_LED_TYPE_LED,   LIBX52_LED_STATE_ON,    (cb_t)libx52_set_led_state},
    {"RED",     ED_LED_TYPE_LED,   LIBX52_LED_STATE_RED,   (cb_t)libx52_set_led_state},
    {"GREEN",   ED_LED_TYPE_LED,   LIBX52_LED_STATE_GREEN, (cb_t)libx52_set_led_state},
    {"AMBER",   ED_LED_TYPE_LED,   LIBX52_LED_STATE_AMBER, (cb_t)libx52_set_led_state},
    // for all leds or mfd only
    {"BLINK",   ED_LED_TYPE_LEDS,  1,                      (cb_t)libx52_set_blink},
    {"NOBLINK", ED_LED_TYPE_LEDS,  0,                      (cb_t)libx52_set_blink},
    {"0%",      ED_LED_TYPE_ALL,   PRCNT(0, 128),          (cb_t)libx52_set_brightness},
    {"5%",      ED_LED_TYPE_ALL,   PRCNT(5, 128),          (cb_t)libx52_set_brightness},
    {"10%",     ED_LED_TYPE_ALL,   PRCNT(10, 128),         (cb_t)libx52_set_brightness},
    {"15%",     ED_LED_TYPE_ALL,   PRCNT(15, 128),         (cb_t)libx52_set_brightness},
    {"20%",     ED_LED_TYPE_ALL,   PRCNT(20, 128),         (cb_t)libx52_set_brightness},
    {"25%",     ED_LED_TYPE_ALL,   PRCNT(25, 128),         (cb_t)libx52_set_brightness},
    {"30%",     ED_LED_TYPE_ALL,   PRCNT(30, 128),         (cb_t)libx52_set_brightness},
    {"35%",     ED_LED_TYPE_ALL,   PRCNT(35, 128),         (cb_t)libx52_set_brightness},
    {"40%",     ED_LED_TYPE_ALL,   PRCNT(40, 128),         (cb_t)libx52_set_brightness},
    {"45%",     ED_LED_TYPE_ALL,   PRCNT(45, 128),         (cb_t)libx52_set_brightness},
    {"50%",     ED_LED_TYPE_ALL,   PRCNT(50, 128),         (cb_t)libx52_set_brightness},
    {"55%",     ED_LED_TYPE_ALL,   PRCNT(55, 128),         (cb_t)libx52_set_brightness},
    {"60%",     ED_LED_TYPE_ALL,   PRCNT(60, 128),         (cb_t)libx52_set_brightness},
    {"65%",     ED_LED_TYPE_ALL,   PRCNT(65, 128),         (cb_t)libx52_set_brightness},
    {"70%",     ED_LED_TYPE_ALL,   PRCNT(70, 128),         (cb_t)libx52_set_brightness},
    {"75%",     ED_LED_TYPE_ALL,   PRCNT(75, 128),         (cb_t)libx52_set_brightness},
    {"80%",     ED_LED_TYPE_ALL,   PRCNT(80, 128),         (cb_t)libx52_set_brightness},
    {"85%",     ED_LED_TYPE_ALL,   PRCNT(85, 128),         (cb_t)libx52_set_brightness},
    {"90%",     ED_LED_TYPE_ALL,   PRCNT(90, 128),         (cb_t)libx52_set_brightness},
    {"95%",     ED_LED_TYPE_ALL,   PRCNT(95, 128),         (cb_t)libx52_set_brightness},
    {"100%",    ED_LED_TYPE_ALL,   PRCNT(100,128),         (cb_t)libx52_set_brightness},
};

static ed_led_type_t *find_led_type(char *name) {
    ed_led_type_t *led = NULL;

    for (int i = 0; i < sizeof(led_types)/sizeof(led_types[0]); i ++) {
        led = &led_types[i];
        if (!strcasecmp(led->name, name))
            break;
    }

    return led;
}


static ed_led_state_t *find_led_state(char *name) {
    ed_led_state_t *state = NULL;

    for (int i = 0; i < sizeof(led_states)/sizeof(led_states[0]); i ++) {
        state = &led_states[i];
        if (!strcasecmp(state->name, name))
            break;
    }

    return state;
}


ed_led_action_t *ed_led_parse_action(char *ledname, char *ledstate) {
    ed_led_action_t *action = NULL;
    ed_led_type_t *led = NULL;
    ed_led_state_t *state = NULL;

    led = find_led_type(ledname);
    if (!led)
        return NULL;

    if (led->type != ED_LED_TYPE_TEXT) {
        state = find_led_state(ledstate);
        if (!state)
            return NULL;
        if (! (led->type & state->allowed_type))
            return NULL;
    }

    action = malloc(sizeof(ed_led_action_t));
    assert(action); 
    action->led = led;
    action->state = state;

    return action;
}



int ed_led_set(libx52_device *x52dev, ed_led_action_t *action, char *data) {
    return action->led->callback(x52dev, action->led->arg, action->state, data ? data : "");
}

int ed_led_apply(libx52_device *x52dev) {
    return libx52_update(x52dev);
}
