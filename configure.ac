#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([x52mfd], [0.0.1], [maxb@tutamail.com])
AC_CONFIG_SRCDIR([include/x52mfd.h])
AC_CONFIG_HEADERS([include/x52mfd_config.h])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIRS([m4])
AC_PREFIX_DEFAULT([$HOME/.local])
AC_DISABLE_STATIC

X52MFD_MODULES_DIR="${ac_default_prefix}/lib/x52mfd"
AC_DEFINE_UNQUOTED([X52MFD_MODULES_DIR], ["$X52MFD_MODULES_DIR"], [Location of shared modules])

# Checks for programs.
AC_PROG_CC
LT_INIT([dlopen])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

PKG_CHECK_MODULES([LIBX52], [libx52 >= 0.2.1], [], AC_MSG_ERROR([libx52 >= 0.2.1 required (get it here: https://github.com/nirenjan/x52pro-linux.git)]))
AC_CHECK_HEADERS([libx52.h libx52/libx52.h])

#AC_CONFIG_SUBDIRS([modules/ed-odyssey
#                   modules/simple-time])

AX_SUBDIRS_CONFIGURE([[modules/simple-time],[modules/ed-odyssey]],[],[[CFLAGS=-I${ac_top_srcdir}/include -I${ac_top_builddir}/include $LIBX52_CFLAGS],[]],[--libdir=${X52MFD_MODULES_DIR}],[])

AC_OUTPUT

