
#include <stdio.h>

#include "elda.h"

static variable_t *variables;
static event_t *events;


char *conf_find_var(char *name) {
    return variables->name;
}

event_t *conf_find_event(EVENT_TYPE type, char *pattern) {

    return events;
}

bool conf_read_file(char *fpath, char **err) {

    // read file by lines

    // save vars

    // save events

    return 0;
}
