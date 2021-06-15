
#ifndef __HAVE_X52MFD_H__
#define __HAVE_X52MFD_H__

#include "x52mfd_config.h"

#if defined(HAVE_LIBX52_H)
#include <libx52.h>
#elif defined(HAVE_LIBX52_LIBX52_H)
#include <libx52/libx52.h>
#endif

typedef int (*x52mfd_func_t)(void);



#endif //__HAVE_X52MFD_H__

