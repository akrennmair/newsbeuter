#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(newsbeuter, 0.4, ak-newsbeuter@synflood.at)
AC_CONFIG_SRCDIR([newsbeuter.cpp])
AC_CONFIG_HEADER([sysconf.h])

# Checks for programs.
AC_PROG_CXX
#AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([curl], [curl_global_init])
AC_CHECK_LIB([mrss], [ mrss_parse_url])
AC_CHECK_LIB([ncursesw], [wgetch])
AC_CHECK_LIB([nxml], [nxml_parse_url])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([sqlite3], [sqlite3_open])
AC_CHECK_LIB([stfl], [stfl_create])
AC_CHECK_LIB([intl], [bindtextdomain])

AM_ICONV


# Checks for header files.
#AC_HEADER_DIRENT
#AC_HEADER_STDC
#AC_CHECK_HEADERS([fcntl.h langinfo.h libintl.h locale.h string.h sys/param.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_HEADER_STDBOOL
#AC_C_CONST
#AC_C_INLINE
#AC_TYPE_PID_T
#AC_TYPE_SIZE_T
#AC_HEADER_TIME
#AC_STRUCT_TM

# Checks for library functions.
#AC_FUNC_ALLOCA
#AC_FUNC_CLOSEDIR_VOID
#AC_FUNC_ERROR_AT_LINE
#AC_FUNC_MKTIME
#AC_TYPE_SIGNAL
#AC_FUNC_STAT
#AC_FUNC_STRFTIME
#AC_CHECK_FUNCS([getcwd gettimeofday memset mkdir nl_langinfo setlocale strchr strerror strrchr strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
