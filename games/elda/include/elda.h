
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
    void        *data;
    struct list *next;
} list_t;

// configured variables
typedef struct {
    char *name;
    char *value;
} variable_t;

// action types
typedef enum {
    X52_ACTION,
    XDO_ACTION,
} ACTION_TYPE;

// action
typedef struct {
    ACTION_TYPE type;
    char        *action;
} action_t;

// event types
typedef enum {
    JOURNAL_EVENT,
    JOYOUT_EVENT,
} EVENT_TYPE;

// event
typedef struct {
    EVENT_TYPE  type;
    pcre        *pattern;
    list_t      *actions;
} event_t;

// logger
void plog(char *fmt, ...);

// read and parse config file
int conf_read_file(char *fpath);
char *conf_find_var(char *name);
event_t *conf_find_event(EVENT_TYPE type, char *pattern);

// get vars and events
char *conf_find_var(char *name);
event_t *conf_match_event(EVENT_TYPE type, char *evstring, char ***subs, int *subs_num);

// events loop
bool events_loop(void);

// actions
bool exec_actions(list_t *aclist, char *buf, char **subs, int subs_num);

// exec xdo actions
bool call_xdo(char *str);

#endif //ELDA_H_INCLUDED


