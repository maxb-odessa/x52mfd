
ACLOCAL_AMFLAGS	=	-I m4

bin_PROGRAMS	=	x52mfd

#include_HEADERS	=	include/x52mfd.h

x52mfd_SOURCES	=	src/main.c \
					include/x52mfd.h

x52mfd_CFLAGS	=	-Wall -Werror -I$(top_srcdir)/include $(X52LIB_CFLAGS)
x52mfd_LDFLAGS	=	-Wl,--no-as-needed
x52mfd_LDADD	=	$(LIBX52_LIBS) -ldl

SUBDIRS			=	$(subdirs_extra)

DIST_SUBDIRS	=	$(SUBDIRS)
