/*
   parse actions json file and prepare it
   */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

#include "cJSON.h"
#include "ed-odyssey.h"


// add new action elem to actions list
ed_pattern_t *add_pattern(ed_pattern_t **patterns, char *pattern) {
    const char *errstr;
    int errpos;
    ed_pattern_t *pattern_elem;

    pattern_elem = calloc(1, sizeof(ed_pattern_t));
    assert(pattern_elem);

    pattern_elem->string = strdup(pattern);
    assert(pattern_elem->string);
    pattern_elem->regex = pcre_compile(pattern, PCRE_DOLLAR_ENDONLY|PCRE_BSR_ANYCRLF, &errstr, &errpos, NULL);
    if (! pattern_elem->regex) {
        fprintf(stderr, "Broken regex '%s' near char %d: %s\n", pattern, errpos, errstr);
        return NULL;
    }

    // add to list head
    pattern_elem->next = *patterns;
    *patterns = pattern_elem;

    return pattern_elem;
}

// add new action to actions list
ed_led_action_t *add_led_action(ed_led_action_t **actions, char *key, char *val) {
    ed_led_action_t *action_elem = ed_led_parse_action(key, val);

    if (! *actions)
        *actions = action_elem;
    else {
        ed_led_action_t *ap = *actions;
        while (ap->next)
            ap = ap->next;
        ap->next = action_elem;
    }

    return action_elem;
}


// read file into mem
static char *read_file(char *fname) {
    char    *fbuf;
    size_t  fbuf_len;
    FILE    *fp;

    // open actions file
    fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open '%s': %s\n", fname, strerror(errno));
        return NULL;
    }

    // allocate buf for entire actions file
    fseek(fp, 0L, SEEK_END);
    fbuf_len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);	
    fbuf = (char*)calloc(fbuf_len + 1, sizeof(char));
    if (! fbuf) {
        fprintf(stderr, "calloc(%ld) failed: %s\n", fbuf_len, strerror(errno));
        fclose(fp);
        return NULL;
    }

    // read actions from file into buffer
    fread(fbuf, sizeof(char), fbuf_len, fp);
    fclose(fp);

    return fbuf;
}



// read and parse actions file
int parse_patterns_file(char *fname, ed_pattern_t **patterns) {
    cJSON *json;
    char *buf;

    // read actions file
    buf = read_file(fname);
    if (! buf)
        return 1;

    // parse actions json file
    json = cJSON_Parse(buf);
    if (! json) {
        fprintf(stderr, "Error in json file '%s':\n%s\n", fname, cJSON_GetErrorPtr());
        return 1;
    }

    // iterate over patterns and actions 
    json = json->child;
    while (json) {

        cJSON *jp = NULL;
        ed_pattern_t    *ed_pattern;
        ed_led_action_t *ed_led_action;

        // non-array value?
        if (json->type != cJSON_Array) {
            fprintf(stderr, "Error in json file '%s': ARRAY expected near line '%s'\n", fname, json->string);
            return 1;
        }

        // pattern - add it to the list
        ed_pattern = add_pattern(patterns, json->string);
        if (! ed_pattern) {
            return 1;
        }

        cJSON_ArrayForEach(jp, json) {
            cJSON *j = jp->child;
            if (j->type != cJSON_String) {
                fprintf(stderr, "Error in json file '%s': STRING expected near line '%s'\n", fname, j->string);
                return 1;
            }
            ed_led_action = add_led_action(&ed_pattern->actions, j->string, j->valuestring);
            if (! ed_led_action) {
                fprintf(stderr, "Invalid action: '%s':'%s'\n", j->string, j->valuestring);
                return 1;
            }
        }

        // take next pattern
        json = json->next;

    } // while(json)


    return 0;
}


// match event string over regex pattern
ed_pattern_t *pattern_match_event(char *event_string, ed_pattern_t *patterns) {
    ed_pattern_t *pattern = patterns;
    int ovector[30];
    int res = 0;

    while (pattern) {
        res = pcre_exec(pattern->regex, 
                NULL,
                event_string,
                strlen(event_string),
                0,
                0,
                ovector,
                30);
        if (res >= 0)
            break;
        pattern = pattern->next;
    }

    // subpatterns matched - extract them
    if (res > 0)
        pcre_get_substring_list(event_string, ovector, res, (const char ***)&pattern->subst);

    return pattern;
}


// parse event string (json) and replace all patterns in 'patt' with values
// replace $1..$9 occurences with submatches
static char *subst_event_string(char *patt, char **subst) {
    char *result;
    char *pptr = patt;
    char *rptr;
    int c, c1;
    // dirty hack below: I don't want to call atoi() on very limited int range
    int I[58];
    I['0'] = 0;
    I['1'] = 1;
    I['2'] = 2;
    I['3'] = 3;
    I['4'] = 4;
    I['5'] = 5;
    I['6'] = 6;
    I['7'] = 7;
    I['8'] = 8;
    I['9'] = 9;

    // nothing to subst? return original
    if (! subst || ! *subst)
        return strdup(patt);

    // guess max result str len and alloc space for it
    result = calloc(strlen(subst[0]) + strlen(patt) + 1, sizeof(char));
    rptr = result;
    assert(result);

    // walk and copy/replace
    while (*pptr) {

        c = *pptr;
        c1 = *(pptr + 1);
        if (c == '$' && isdigit(c1)) {
            if (subst[I[c1]]) {
                strcat(result, subst[I[c1]]);
                pptr += 2;
                rptr += strlen(subst[I[c1]]);
                continue;
            }
        }

        *(rptr ++) = *(pptr ++);
    }

    return result;
}


// apply pattern actions
int pattern_apply_actions(x52mfd_t *x52mfd, ed_pattern_t *pattern, char *event_string) {
    ed_led_action_t *action = pattern->actions;

    if (debug) {
        fprintf(stderr, "GOT: '%s'\nMATCHED: '%s'\n", event_string, pattern->string);
        if (pattern->subst) {
            fprintf(stderr, "SUBMATRCHES:\n");
            for (int i = 0; pattern->subst[i]; i ++ )
                printf("  $%d => '%s'\n", i, pattern->subst[i]);
        }
    }

    while (action) {
        char *text = NULL;

        if (action->led->type == ED_LED_TYPE_TEXT) {
            text = subst_event_string(action->state->patt, pattern->subst);
            if (debug)
                fprintf(stderr, "SUBSTED TEX: '%s'\n", text);
        }

        ed_led_set(x52mfd->dev, action, text);

        if (text)
            free(text);

        action = action->next;
    }

    if (pattern->subst) {
        pcre_free_substring_list((const char **)pattern->subst);
        pattern->subst = NULL;
    }

    ed_led_apply(x52mfd->dev);

    return 0;
}


