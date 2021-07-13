ACLOCAL_AMFLAGS			=	-I m4

AM_CPPFLAGS				=	-D_ED_ODYSSEY_DEF_PATTERNS_PATH=\"$(sysconfdir)/ed-odyssey.json\"
EXTRA_DIST				=	ed-odyssey.json

bin_PROGRAMS			=	elda

eldadir					=	$(sysconfdir)
elda_DATA				=	$(EXTRA_DIST)

elda_SOURCES			=	src/elda.c

#							ed-odyssey.h	\
#							led_actions.c	\
#							patterns.c		\
#							journal.c		\
							cJSON.h			\
							cJSON.c

elda_CFLAGS				=	-Wall $(PCRE_CFLAGS)
elda_LDFLAGS			=
elda_LDADD				=	$(PCRE_LIBS)


