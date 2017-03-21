dnl                                                -*- Autoconf -*-
dnl  Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([goofy], [0.0.1], [dacav at openmailbox dot org])

AM_INIT_AUTOMAKE([1.15])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([src/config.h])

dnl  Checks for programs.
AC_PROG_CXX

dnl  Checks for libraries.
AC_CHECK_LIB([magic], [magic_open])
AC_CHECK_LIB([event], [event_new])

dnl  Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h sys/socket.h unistd.h])

dnl  Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T

dnl  Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memchr memset strerror strstr strtoul])

AC_CONFIG_FILES([Makefile] [src/Makefile])
AC_OUTPUT