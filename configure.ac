dnl                                                -*- Autoconf -*-
dnl  Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([goofy], [0.0.1], [dacav at openmailbox dot org])

AM_INIT_AUTOMAKE([1.15] [foreign] [silent-rules])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([src/config.h])

dnl  Checks for programs.
AC_PROG_CXX

dnl  Checks for libraries.
AC_SEARCH_LIBS([magic_open], [magic], [],
    AC_MSG_ERROR([Please install libmagic])
)
AC_SEARCH_LIBS([event_new], [event], [],
    AC_MSG_ERROR([Please install libevent2])
)

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

AC_CONFIG_FILES([Makefile] [src/Makefile] [tests/Makefile])
AC_OUTPUT
