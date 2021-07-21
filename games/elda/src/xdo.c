
#include "elda.h"

#ifdef WITH_XDO

bool call_xdo(char *str) {
    plog("xdo: <%s>\n", str);
    return true;
}

#endif
