#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([include/x52mfd.h])
AC_CONFIG_HEADERS([include/config.h])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
LT_INIT([dlopen])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

PKG_CHECK_MODULES([LIBX52], [libx52 >= 0.2.1], [], AC_MSG_ERROR([libx52 >= 0.2.1 required (get it here: https://github.com/nirenjan/x52pro-linux.git)]))
AC_CHECK_HEADERS([libx52.h libx52/libx52.h])

AC_CONFIG_SUBDIRS([modules/ed-odyssey
                   modules/simple-time])
AC_OUTPUT
