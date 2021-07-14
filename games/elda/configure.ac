#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([elda], [0.0.1], [maxb@tutamail.com])
AC_CONFIG_SRCDIR([src/elda.c])
AC_CONFIG_HEADERS([include/config.h])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([subdir-objects])

CFLAGS="-std=gnu11 -g -ggdb -O0 -Wall -Werror"

# Checks for programs.
AC_PROG_CC


# Checks for header files.
AC_CHECK_HEADER_STDBOOL
AC_CHECK_HEADERS([stdarg.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Checks for libraries.
PKG_CHECK_MODULES([PCRE], [libpcre >= 8.0.0], [], AC_MSG_ERROR([pcre >= 8.0.0 required]))

AC_CHECK_HEADERS([xdo.h], [], [AC_MSG_ERROR(['libxdo-devel' required, please install it])])
AC_CHECK_LIB([xdo], [xdo_new], [], [AC_MSG_ERROR(['libxdo' required, please install it])])


AC_OUTPUT

