
#ifndef ELDA_H_INCLUDED
#define ELDA_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#ifdef WITH_XDO
    XDO_ACTION,
#endif
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

// events polling timeout
#define EVENT_TIMEOUT_MS   30
#define EVENT_TIMEOUT_US   (EVENT_TIMEOUT_MS * 1000)

// logger
void plog(char *fmt, ...);

// read and parse config file
int conf_read_file(char *fpath);
char *conf_find_var(char *name);
event_t *conf_find_event(EVENT_TYPE type, char *pattern);
char *strip_string(char *str);

// get vars and events
char *conf_find_var(char *name);
event_t *conf_match_event(EVENT_TYPE type, char *evstring, char ***subs, int *subs_num);

// events loop
bool events_loop(void);

// joyout events handler
bool joyout_events_init(void);
bool joyout_event_get(char **bufp);

// journal events handler
bool journal_events_init(void);
bool journal_event_get(char **bufp);

// actions
bool exec_actions(list_t *aclist, char *buf, char **subs, int subs_num);

#ifdef WITH_XDO
// exec xdo actions
bool call_xdo(char *str);
#endif

#endif //ELDA_H_INCLUDED


