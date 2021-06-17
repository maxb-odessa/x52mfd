
#ifndef __HAVE_X52MFD_H__
#define __HAVE_X52MFD_H__

#include "x52mfd_config.h"

#if defined(HAVE_LIBX52_H)
#include <libx52.h>
#elif defined(HAVE_LIBX52_LIBX52_H)
#include <libx52/libx52.h>
#endif

// modules loading and calling stuff
typedef int (*x52mfd_func_t)(const char **errstr);

#define XSTR(X) STR(X)
#define STR(X) #X

#define X52MFD_INIT_FUNC    x52mfd_init
#define X52MFD_RUN_FUNC     x52mfd_run
#define X52MFD_FINISH_FUNC  x52mfd_finish

#define X52MFD_INIT(funcname)   int X52MFD_INIT_FUNC(const char **err) { return funcname(err); }
#define X52MFD_RUN(funcname)    int X52MFD_RUN_FUNC(const char **err) { return funcname(err); }
#define X52MFD_FINISH(funcname) int X52MFD_FINISH_FUNC(const char **err) { return funcname(err); }

#endif //__HAVE_X52MFD_H__

