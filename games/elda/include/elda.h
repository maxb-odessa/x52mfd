
#ifndef ELDA_H_INCLUDED
#define ELDA_H_INCLUDED

#include "config.h"

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#else
#define bool int
#define true 1
#define false 0
#endif

#include <pcre.h>

// simple list node
typedef struct list {
    void *data;
    struct list *next;
} list_t;


// configured variables
typedef struct {
    char *name;
    char *value;
} variable_t;

// xdo action
typedef char *action_xdo_t;

// x52pro action
typedef char *action_x52_t;

// event types
typedef enum {
    JOURNAL_EVENT,
    BUTTON_EVENT,
} EVENT_TYPE;


// event
typedef struct {
    EVENT_TYPE type;
    union {
        pcre *journal;
        char *button;
    } pattern;
    list_t *x52_actions;
    list_t *xdo_actions;
} event_t;

// logger
void plog(char *fmt, ...);

// read and parse config file
int conf_read_file(char *fpath);
char *conf_find_var(char *name);
event_t *conf_find_event(EVENT_TYPE type, char *pattern);

// get vars and events
char *conf_find_var(char *name);
event_t *conf_match_event(EVENT_TYPE type, char *evstring, char **subs);



#endif //ELDA_H_INCLUDED


