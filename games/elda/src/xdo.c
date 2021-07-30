
#include "elda.h"

#include <stdio.h>

#ifdef WITH_XDO

static FILE *fp;
bool call_xdo(char *str) {
    const char *fname = "/home/vesemir/xdo-elda.log";

    if (! fp)
        fp = fopen(fname, "w+");

    fprintf(fp, str);
    fflush(fp);

    //plog("xdo: <%s>\n", str);
    return true;
}

#endif
