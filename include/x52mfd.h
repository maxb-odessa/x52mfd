
#ifndef __HAVE_X52MFD_H__
#define __HAVE_X52MFD_H__

#include "x52mfd_config.h"

#include <libx52/libx52.h>


typedef struct {
    libx52_device   *dev;
} x52mfd_t;


// modules loading and calling stuff
typedef int (*x52mfd_func_t)(x52mfd_t *);

#define XSTR(X) STR(X)
#define STR(X) #X

#define X52MFD_INIT_FUNC    x52mfd_init
#define X52MFD_RUN_FUNC     x52mfd_run
#define X52MFD_FINISH_FUNC  x52mfd_finish

#define X52MFD_INIT(funcname)   int X52MFD_INIT_FUNC(x52mfd_t *x52mfd) { return funcname(x52mfd); }
#define X52MFD_RUN(funcname)    int X52MFD_RUN_FUNC(x52mfd_t *x52mfd) { return funcname(x52mfd); }
#define X52MFD_FINISH(funcname) int X52MFD_FINISH_FUNC(x52mfd_t *x52mfd) { return funcname(x52mfd); }

int x52mfd_init(x52mfd_t *x52mfd);
void x52mfd_can(x52mfd_t *x52mfd);
int x52mfd_disconnect(x52mfd_t *x52mfd);

#endif //__HAVE_X52MFD_H__

