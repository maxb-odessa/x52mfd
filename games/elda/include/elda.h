
#ifndef ELDA_H_INCLUDED
#define ELDA_H_INCLUDED

#include "config.h"

#ifdef HAVE_STDBOOL_H1
#include <stdbool.h>
#else
typedef int bool;
const int true = 1;
const int false  = 0;
#endif

#include <pcre.h>


// configured variables
typedef struct variable {
    char *name;
    char *value;
    struct variable *next;
} variable_t;


// x52 actions
typedef struct action_x52 {
    char *action;
    struct action_x52 *next;
} action_x52_t;


// xdo actions
typedef struct action_xdo {
    char *action;
    struct action_xdo *next;
} action_xdo_t;


// event types
typedef enum {
    JOURNAL,
    BUTTON,
} EVENT_TYPE;


// events
typedef struct event {
    EVENT_TYPE type;
    union {
        pcre *journal;
        char *button;
    } pattern;
    action_x52_t *x52_actions;
    action_xdo_t *xdo_actions;
    struct action *next;
} event_t;


// read and parse config file
int conf_read_file(char *fpath, char **err);
char *conf_find_var(char *name);
event_t *conf_find_event(EVENT_TYPE type, char *pattern);


#endif //ELDA_H_INCLUDED
