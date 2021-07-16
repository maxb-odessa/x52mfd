
#include "elda.h"

#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>

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
event_t *conf_match_event(EVENT_TYPE type, char *evstring, char ***subs, int *subs_num) {
    list_t *lp = events;
    event_t *evp;

    // search the list
    while (lp) {
        evp = (event_t *)lp->data;

        // match event type
        if (evp->type == type) {
            int ovector[30];
            int rc = pcre_exec(evp->pattern, NULL, evstring, strlen(evstring), 0, 0, ovector, 30);
            if (rc >= 0) {
                *subs_num = rc;
                if (rc > 0)
                    pcre_get_substring_list(evstring, ovector, rc, (const char ***)subs);
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
    variables = lp;

    return (variable_t *)lp->data;
}

// compose and add new event to the list
static event_t *add_event(EVENT_TYPE type, char *pattern) {
    list_t *lp;
    event_t *evp;
    const char *errstr;
    int errpos;

    // check
    if (! pattern)
        return NULL;

    // create new event entry
    evp = (event_t *)calloc(1, sizeof(event_t));
    assert(evp);

    // init event entry
    evp->type = type;
    evp->pattern = pcre_compile(pattern, PCRE_DOLLAR_ENDONLY|PCRE_BSR_ANYCRLF, &errstr, &errpos, NULL);
    if (! evp->pattern) {
        plog("broken regex '%s' near char %d: %s\n", pattern, errpos, errstr);
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
    events = lp;

    return evp;
}


// add actions to event entry
// will add to actions list tail - actions order is important
static list_t *add_action(event_t *evp, ACTION_TYPE actype, char *action) {
    list_t *lp, *ac;

    // check
    if (evp == NULL) {
        plog("got action but no event defined for it yet\n");
        return NULL;
    }

    // make action entry
    ac = (list_t *)calloc(1, sizeof(list_t));
    assert(ac);
    ac->data = malloc(sizeof(action_t));
    assert(ac->data);
    ((action_t *)ac->data)->type = actype;
    ((action_t *)ac->data)->action = strdup(action);

    // the list is empty yet - make it
    if (evp->actions == NULL) 
        evp->actions = ac;
    else {
        // add to the tail if not
        lp = evp->actions;
        while (lp->next)
            lp = lp->next;
        lp->next = ac;
    }

    // done
    return ac;
}

// parsed line types
typedef enum {
    CONF_ENTRY_TYPE_INVALID,
    CONF_ENTRY_TYPE_EMPTY,
    CONF_ENTRY_TYPE_VAR,
    CONF_ENTRY_TYPE_EVENT,
    CONF_ENTRY_TYPE_ACTION,
} CONF_ENTRY_TYPE;

// strip string form leading and trailing blanks
static char *strip_string(char *str) {
    char *sp = str;
    size_t slen;

    // check
    if (! str)
        return NULL;

    slen = strlen(str);

    // check two
    if (slen == 0)
        return sp;

    // strip trailing blanks
    sp = str + slen - 1;
    while (sp != str && isspace(*sp))
        *sp -- = '\0';

    // strip leading blanks
    sp = str;
    while (*sp && isspace(*sp))
        sp ++;

    return sp;
}

// parse config line
static CONF_ENTRY_TYPE parse_line(char *buf, EVENT_TYPE *evtype, ACTION_TYPE *actype, char *tokens[2]) {
    char *bufp, *pp;

    tokens[0] = tokens[1] = NULL;

    // check
    if (buf == NULL)
        return CONF_ENTRY_TYPE_EMPTY;

    // strip buf from surrounding blanks
    bufp = strip_string(buf);

    // check for empty line
    if (*bufp == '\0')
        return CONF_ENTRY_TYPE_EMPTY;

    // skip comments
    if (*bufp == '#' || *bufp == ';')
        return CONF_ENTRY_TYPE_EMPTY;

    // examine line type
    pp = strchr(bufp, ':');
    if (pp == NULL) {
        plog("broken line '%s'\n", bufp);
        return CONF_ENTRY_TYPE_INVALID;
    }

    // advance to next token
    do {
        *pp ++ = '\0';
    } while (*pp && isspace(*pp));

    // check
    if (*pp == '\0') {
        plog("icomplete '%s' token\n", bufp);
        return CONF_ENTRY_TYPE_INVALID;
    }

    // guess line type: variable, event or action
    if (! strcmp("var", bufp)) {

        // save var name and advance to its (possible) value
        tokens[0] = pp;
        while (*pp && !isspace(*pp))
            pp ++;

        // save var value (optional)
        while (*pp && isspace(*pp))
            *pp ++ = '\0';
        tokens[1] = pp;

        return CONF_ENTRY_TYPE_VAR;

    } else if (! strcmp("journal", bufp)) {

        tokens[0] = pp;
        *evtype = JOURNAL_EVENT;
        return CONF_ENTRY_TYPE_EVENT;

    } else if (! strcmp("joyout", bufp)) {

        tokens[0] = pp;
        *evtype = JOYOUT_EVENT;
        return CONF_ENTRY_TYPE_EVENT;

    } else if (! strcmp("x52", bufp)) {

        tokens[0] = pp;
        *actype = X52_ACTION;
        return CONF_ENTRY_TYPE_ACTION;

    } else if (! strcmp("xdo", bufp)) {

        tokens[0] = pp;
        *actype = XDO_ACTION;
        return CONF_ENTRY_TYPE_ACTION;

    }

    // unknown line type
    plog("unknown type '%s'\n", bufp);
    return CONF_ENTRY_TYPE_INVALID;
}

// read and parse config file
bool conf_read_file(char *confpath) {
    FILE *fp;
    char buf[8192]; // we expect no long lines in config file
    char *tokens[2];
    event_t *evp = NULL;
    EVENT_TYPE evtype;
    ACTION_TYPE actype;
    int lineno = 0;
    int rc = true;

    // open config file
    fp = fopen(confpath, "r");
    if (! fp) {
        plog("failed to open config file '%s': %s\n", confpath, strerror(errno));
        return false;
    }

    // read file by lines
    while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {

        lineno ++;

        // parse config line
        switch (parse_line(buf, &evtype, &actype, tokens)) {

            // comments and empty lines
            case CONF_ENTRY_TYPE_EMPTY :
                break;

                // save vars
            case CONF_ENTRY_TYPE_VAR :
                if (add_variable(tokens[0], tokens[1]) == NULL)
                    rc = false;
                break;

                // save events
            case CONF_ENTRY_TYPE_EVENT :
                evp = add_event(evtype, tokens[0]);
                if (evp == NULL)
                    rc = false;
                break;

                // save event action
            case CONF_ENTRY_TYPE_ACTION :
                if (add_action(evp, actype, tokens[0]) == NULL)
                    rc = false;
                break;

            case CONF_ENTRY_TYPE_INVALID :
            default :
                rc = false;
                break;
        }

        // parse failed?
        if (rc == false) {
            plog("parse of config '%s' failed at line %d\n", confpath, lineno);
            break;
        }

    }

    // done
    return rc;
}

