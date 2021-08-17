#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([x52mfd],[0.1.0],[maxb@tutamail.com])
AC_CONFIG_SRCDIR([include/x52mfd.h])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIRS([m4])
AC_PREFIX_DEFAULT([$HOME/.local])
AM_INIT_AUTOMAKE([subdir-objects])

CFLAGS="-std=gnu11 -g -ggdb -O0 -Wall -Werror"

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AX_PTHREAD
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="-D_GNU_SOURCE $CFLAGS $PTHREAD_CFLAGS"
CC="$PTHREAD_CC"

# Checks for header files.
AC_CHECK_HEADERS([pthread.h stdarg.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

PKG_CHECK_MODULES([LIBX52], [libx52 >= 1.0.0], [], AC_MSG_ERROR([libx52 >= 1.0.0 required (get it here: https://github.com/nirenjan/x52pro-linux.git)]))
PKG_CHECK_MODULES([LIBX52IO], [libx52io >= 1.0.0], [], AC_MSG_ERROR([libx52io >= 1.0.0 required (get it here: https://github.com/nirenjan/x52pro-linux.git)]))


AC_CONFIG_SUBDIRS([games/elda])

AC_OUTPUT

