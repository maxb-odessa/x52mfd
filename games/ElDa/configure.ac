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

CFLAGS="-std=c11 -g -ggdb -O0"

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([PCRE], [libpcre >= 8.0.0], [], AC_MSG_ERROR([pcre >= 8.0.0 required]))


# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.



AC_OUTPUT
