ACLOCAL_AMFLAGS			=	-I m4

AM_CPPFLAGS				=	-D_ED_ODYSSEY_DEF_PATTERNS_PATH=\"$(sysconfdir)/ed-odyssey.json\"
EXTRA_DIST				=	ed-odyssey.json

lib_LTLIBRARIES			=	ed-odyssey.la

ed_odyssey_ladir		=	$(sysconfdir)
ed_odyssey_la_DATA		=	$(EXTRA_DIST)

ed_odyssey_la_SOURCES	=	ed-odyssey.c	\
							ed-odyssey.h	\
							led_actions.c	\
							patterns.c		\
							journal.c		\
							cJSON.h			\
							cJSON.c

ed_odyssey_la_CFLAGS	=	-Wall $(PCRE_CFLAGS)
ed_odyssey_la_LDFLAGS	=	-module -avoid-version
ed_odyssey_la_LIBADD	=	$(PCRE_LIBS)


