
#include "elda.h"

#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include <assert.h>

// list of configured variables
static list_t *variables;

// list of configured events
static list_t *events;


// search for configured variable and return its value (or NULL)
// also search env vars if not found in configured vars list
char *conf_find_var(char *name) {
    list_t *lp = variables;
    variable_t *vp;

    // search the list
    while (lp) {
        vp = (variable_t *)lp->data;
        if (! strcmp(name, vp->name))
            return vp->value;
        lp = lp->next;
    }

    // now try to search env vars
    return getenv(name);
}

// find matching event and return its struct (or NULL)
// also will fill '**subs' array with matched subpatterns in case of regex
// 'subs' must be freed by caller
event_t *conf_match_event(EVENT_TYPE type, char *evstring, char **subs) {
    list_t *lp = events;;
    event_t *evp;

    // search the list
    while (lp) {
        evp = (event_t *)lp->data;

        // button event case
        if (type == BUTTON_EVENT && evp->type == BUTTON_EVENT) {
            if (! strcmp(evstring, evp->pattern.button))
                return evp;
        } else if (type == JOURNAL_EVENT && evp->type == JOURNAL_EVENT) {
            // journal event case
            int ovector[30];
            int rc = pcre_exec(evp->pattern.journal, NULL, evstring, strlen(evstring), 0, 0, ovector, 30);
            if (rc >= 0) {
                if (rc > 0)
                    pcre_get_substring_list(evstring, ovector, rc, (const char ***)&subs);
                return evp;
            }
        }

        // pick next event pattern
        lp = lp->next;
    }

    // no match
    return NULL;
}

// add new variable to the list
static variable_t *add_variable(char *name, char *value) {
    list_t *lp;

    // check
    if (!name || !value)
        return NULL;

    // create new list node
    lp = (list_t *)calloc(1, sizeof(list_t));
    assert(lp);

    // create new var data
    lp->data = malloc(sizeof(variable_t));
    assert(lp->data);
    ((variable_t *)lp->data)->name = strdup(name);
    ((variable_t *)lp->data)->value = strdup(value);

    // add to list head
    lp->next = variables;

    return (variable_t *)lp->data;
}

// compose and add new event to the list
static event_t *add_event(EVENT_TYPE type, char *pattern) {
    list_t *lp;
    event_t *evp;

    // check
    if (! pattern)
        return NULL;

    // create new event entry
    evp = (event_t *)calloc(1, sizeof(event_t));
    assert(evp);

    // init event entry
    evp->type = type;

    if (type == BUTTON_EVENT) {
        evp->pattern.button = strdup(pattern);
    } else if (type == JOURNAL_EVENT) {
        const char *errstr;
        int errpos;
        evp->pattern.journal = pcre_compile(pattern, PCRE_DOLLAR_ENDONLY|PCRE_BSR_ANYCRLF, &errstr, &errpos, NULL);
        if (! evp->pattern.journal) {
            plog("broken regex '%s' near char %d: %s\n", pattern, errpos, errstr);
            free(evp);
            return NULL;
        }
    } else {
        free(evp);
        return NULL;
    }

    // create new list node
    lp = (list_t *)calloc(1, sizeof(list_t));
    assert(lp);

    // add data to list node
    lp->data = (void *)evp;

    // add to list head
    lp->next = events;

    return evp;
}

// read and parse config file
bool conf_read_file(char *fpath) {

    // read file by lines

    // save vars
    add_variable("aa", "bb");

    // save events
    add_event(JOURNAL_EVENT, "aaa");

    return true;
}
