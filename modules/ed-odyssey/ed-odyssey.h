

#include <pcre.h>

#include "x52mfd.h"

// leds types
enum {
    ED_LED_TYPE_TEXT = 0x01,
    ED_LED_TYPE_LED  = 0x02,
    ED_LED_TYPE_MFD  = 0x04,
    ED_LED_TYPE_LEDS = 0x08,
};

// leds names and ids
typedef struct {
    char    *name;
    int     type;
    int     arg;
    int     (*callback)();
} ed_led_type_t;

// leds and mfd states
typedef struct {
    char    *name;
    int     allowed_type;
    int     state;
    int     (*callback)();
} ed_led_state_t;

// led and its state action
typedef struct ed_led_action {
    ed_led_type_t           *led;
    ed_led_state_t          *state;
    struct ed_led_action    *next;
} ed_led_action_t;

// these should be called within a module
ed_led_action_t *ed_led_parse_action(char *ledname, char *ledstate);
int ed_led_set(libx52_device *x52dev, ed_led_action_t *action, char *data);
int ed_led_apply(libx52_device *x52dev);

// events patterns
typedef struct ed_pattern {
    pcre                *regex;
    ed_led_action_t     *actions;
    struct ed_pattern   *next;
} ed_pattern_t;


// ???
int parse_patterns_file(char *fname, ed_pattern_t **patterns);
