/*
   parse actions json file and prepare it
   */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "cJSON.h"
#include "ed-odyssey.h"


// add new action elem to actions list
ed_pattern_t *add_pattern(ed_pattern_t **patterns, char *pattern) {
    const char *errstr;
    int errpos;
    ed_pattern_t *pattern_elem;

    pattern_elem = calloc(1, sizeof(ed_pattern_t));
    assert(pattern_elem);

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
        while ((*actions)->next)
            *actions = (*actions)->next;
        (*actions)->next = action_elem;
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
        fprintf(stderr, "Failed to open %s: %s", fname, strerror(errno));
        return NULL;
    }

    // allocate buf for entire actions file
    fseek(fp, 0L, SEEK_END);
    fbuf_len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);	
    fbuf = (char*)calloc(fbuf_len + 1, sizeof(char));
    if (! fbuf) {
        fprintf(stderr, "calloc(%ld) failed:: %s", fbuf_len, strerror(errno));
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
        fprintf(stderr, "Error in json file '%s':\n%s", fname, cJSON_GetErrorPtr());
        return 1;
    }

    // iterate over patterns and actions 
    json = json->child;
    while (json) {

        cJSON *js_pattern = NULL;
        ed_pattern_t    *ed_pattern;
        ed_led_action_t *ed_led_action;

        // non-array value?
        if (json->type != cJSON_Array) {
            fprintf(stderr, "Error in json file '%s': ARRAY expected near line '%s'", fname, json->string);
            return 1;
        }

        // pattern - add it to the list
        ed_pattern = add_pattern(patterns, json->string);
        if (! ed_pattern) {
            return 1;
        }

        cJSON_ArrayForEach(js_pattern, json->child) {

            if (js_pattern->type != cJSON_String) {
                fprintf(stderr, "Error in json file '%s': STRING expected near line '%s'", fname, json->string);
                return 1;
            }

            ed_led_action = add_led_action(&ed_pattern->actions, js_pattern->string, js_pattern->valuestring);
            if (! ed_led_action) {
                fprintf(stderr, "Invalid action: '%s':'%s'\n", js_pattern->string, js_pattern->valuestring);
                return 1;
            }

        }

        // take next pattern
        json = json->next;

    } // while(json)


    return 0;
}



