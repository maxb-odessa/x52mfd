
#include "elda.h"


// find event and execute it
static bool process_event(EVENT_TYPE evtype, char *buf) {
    event_t *evp;
    char **subs;
    int subs_num = 0;
    bool ok = true;
    char *bufp;

    bufp = strip_string(buf);

    // match buf over events
    evp = conf_match_event(evtype, bufp, &subs, &subs_num);
    if (! evp)
        return ok;

    // event matched - execute its actions
    ok = exec_actions(evp->actions, bufp, subs, subs_num);

    // cleanups
    if (subs_num > 0)
        pcre_free_substring_list((const char **)subs);

    // done
    return ok;
}



// main events listening loop
// read events from journal and stdin and execute associated actions
bool events_loop(void) {
    char *bufp;
    bool ok;

    // init joyout events handlers
    if (! joyout_events_init()) {
        plog("failed to init joyout events handler\n");
        return false;
    }

    // init journal events handlers
    if (! journal_events_init()) {
        plog("failed to init journal events handler\n");
        return false;
    }

    // poll for events
    while (true) {

        // get event string from stdin (joyout) and process it
        bufp = NULL;
        if (! (ok = joyout_event_get(&bufp)))
            break;
        if (bufp && ! (ok = process_event(JOYOUT_EVENT, bufp)))
            break;

        // get event string from journal and process it
        bufp = NULL;
        if (! (ok = journal_event_get(&bufp)))
            break;
        if (bufp && ! (ok = process_event(JOURNAL_EVENT, bufp)))
            break;

        //    plog("bufp: <%s>\n", bufp);
    }

    // we're done
    return ok;
}


