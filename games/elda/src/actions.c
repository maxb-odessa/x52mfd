
#include "elda.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

// examine event string and replace all patterns in it with values from 'subs'
// replace $1..$9 occurences with submatches
// $0 = whole matched string
static char *replace_subs(char *patt, char **subs, int nsubs) {
    char *result;
    char *pptr = patt;
    char *rptr;
    int c, c1;
    // dirty hack below: I don't want to call atoi() on a very limited ints range
    int INT[] = {
        ['0'] = 0,
        ['1'] = 1,
        ['2'] = 2,
        ['3'] = 3,
        ['4'] = 4,
        ['5'] = 5,
        ['6'] = 6,
        ['7'] = 7,
        ['8'] = 8,
        ['9'] = 9,
    };

    // nothing to subst? return original
    if (! subs || ! *subs || nsubs < 1)
        return strdup(patt);

    // guess max result str len and alloc space for it
    result = calloc(strlen(subs[0]) + strlen(patt) + 1, sizeof(char));
    rptr = result;
    assert(result);

    // walk and copy/replace
    while (*pptr) {

        c = *pptr;
        c1 = *(pptr + 1);
        if (c == '$' && isdigit(c1)) {
            if (subs[INT[c1]]) {
                strcat(result, subs[INT[c1]]);
                pptr += 2;
                rptr += strlen(subs[INT[c1]]);
                continue;
            }
        }

        *(rptr ++) = *(pptr ++);
    }

    return result;
}


// execute event associated action
bool exec_actions(list_t *aclist, char *buf, char **subs, int subs_num) {
    list_t *lp = aclist;
    action_t *ap;
    char *preped;

    // walk through configured actions and execute them
    while (lp) {
        ap = (action_t *)lp->data;
        //plog("=> %d <%s>\n", ap->type, ap->action);
        preped = replace_subs(ap->action, subs, subs_num);
        switch (ap->type) {
            case X52_ACTION :
                puts(preped); // this is simple: just send x52 action string to stdout
                fflush(stdout);
                break;
#ifdef WITH_XDO
            case XDO_ACTION :
                call_xdo(preped);
                break;
#endif
            default :
                break;
        }
        free(preped);
        lp = lp->next;
    }

    // we're done here
    return true;
}

