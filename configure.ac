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
PKG_PROG_PKG_CONFIG

dnl  Checks for libraries.
AC_SEARCH_LIBS([magic_open], [magic], [],
    AC_MSG_ERROR([Please install libmagic])
)

PKG_CHECK_MODULES([event], [libevent >= 2.0.0])
PKG_CHECK_MODULES([config], [libconfig++ >= 1.5])

AC_CHECK_HEADERS([arpa/inet.h fcntl.h sys/socket.h syslog.h unistd.h])

AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memchr memset strerror strstr])

AC_CONFIG_FILES([Makefile] [src/Makefile] [tests/Makefile])
AC_OUTPUT
