ACLOCAL_AMFLAGS			=	-I m4

AM_CPPFLAGS				=	-DDEFAULT_CONFIG_FILE=\"$(sysconfdir)/ed-odyssey.conf\"
EXTRA_DIST				=	ed-odyssey.conf.in \
							x52mfd.service.in

bin_PROGRAMS			=	elda
elda_config				=	ed-odyssey.conf

eldadir					=	$(sysconfdir)
elda_DATA				=	$(elda_config)

servicedir				=	$(HOME)/.config/systemd/user
service_DATA			=	x52mfd.service

elda_SOURCES			=	src/elda.c		\
							src/plog.c		\
							src/conf.c		\
							src/events.c	\
							src/joyout.c	\
							src/journal.c	\
							src/actions.c	\
							src/xdo.c		\
							include/elda.h

elda_CFLAGS				=	-I$(top_srcdir)/include $(PCRE_CFLAGS) $(XDO_CFLAGS)
elda_LDFLAGS			=
elda_LDADD				=	$(PCRE_LIBS) $(XDO_LIBS)


