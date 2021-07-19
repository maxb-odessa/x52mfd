ACLOCAL_AMFLAGS			=	-I m4

AM_CPPFLAGS				=	-DDEFAULT_CONFIG_FILE=\"$(sysconfdir)/ed-odyssey.conf\"
EXTRA_DIST				=	ed-odyssey.conf

bin_PROGRAMS			=	elda

eldadir					=	$(sysconfdir)
elda_DATA				=	$(EXTRA_DIST)

elda_SOURCES			=	src/elda.c		\
							src/conf.c		\
							src/plog.c		\
							src/events.c	\
							src/actions.c	\
							src/xdo.c		\
							src/cJSON.c		\
							include/elda.h	\
							include/cJSON.h

elda_CFLAGS				=	-I$(top_srcdir)/include $(PCRE_CFLAGS) $(XDO_CFLAGS)
elda_LDFLAGS			=
elda_LDADD				=	$(PCRE_LIBS) $(XDO_LIBS)


