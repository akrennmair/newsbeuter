#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_include(m4/ac_prog_cococpp.m4)

AC_PREREQ(2.59)
AC_INIT(newsbeuter, 0.8, ak-newsbeuter@synflood.at)
AC_CONFIG_SRCDIR([newsbeuter.cpp])
AC_CONFIG_HEADER([sysconf.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_COCOCPP
AC_CHECK_PROG(PERL,perl,perl)

# Checks for libraries.
AC_CHECK_LIB([curl], [curl_global_init])
AC_CHECK_LIB([mrss], [mrss_parse_url])
AC_CHECK_LIB([ncursesw], [wgetch])
AC_CHECK_LIB([nxml], [nxml_parse_url])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([sqlite3], [sqlite3_open])
AC_CHECK_LIB([stfl], [stfl_create])
AC_CHECK_LIB([intl], [bindtextdomain])

AM_ICONV

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
