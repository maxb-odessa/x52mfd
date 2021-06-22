ACLOCAL_AMFLAGS			=	-I m4

lib_LTLIBRARIES			=	ed-odyssey.la

ed_odyssey_la_SOURCES	=	ed-odyssey.c	\
							ed-odyssey.h	\
							led_actions.c	\
							patterns.c		\
							events.c		\
							cJSON.h			\
							cJSON.c

ed_odyssey_la_CFLAGS	=	-Wall $(PCRE_CFLAGS)
ed_odyssey_la_LDFLAGS	=	-module -avoid-version
ed_odyssey_la_LIBADD	=	$(PCRE_LIBS)

EXTRA_DIST				=	ed-odyssey.json
