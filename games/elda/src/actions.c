
#include "elda.h"

bool exec_actions(list_t *aclist, char *buf, char *subs[]) {
    list_t *lp = aclist;
    action_t *ap;

    plog("AC: %s\n", buf);

    while (lp) {
        ap = (action_t *)lp->data;
        plog("=> %d <%s>\n", ap->type, ap->action);
        lp = lp->next;
    }

    return true;
}

