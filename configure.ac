# -*- mode: autoconf -*-

AC_INIT([freespace2],[3.8.1],)

AM_INIT_AUTOMAKE([-Wno-portability subdir-objects dist-bzip2 nostdinc foreign])
AC_CONFIG_HEADERS([src/_config.hpp:src/config.ac])

AC_CONFIG_MACRO_DIRS([etc/m4])

AC_PROG_OBJC
AC_PROG_CXX
AC_CONFIG_CXX_WARNINGS

AC_ENABLE_CXX_DIALECT([c++1z])
dnl AC_ENABLE_OPENMP

AC_PROG_LIBTOOL
AC_PROG_MAKE_SET(gmake)
AM_PROG_AS

AC_CONFIG_WITH_SDL2([2.0.8])
AC_CONFIG_WITH_RANGE3

BOOST_REQUIRE([1.66.0])

BOOST_SMART_PTR
BOOST_CHRONO
BOOST_DATE_TIME
BOOST_FILESYSTEM
BOOST_LOG
BOOST_PROGRAM_OPTIONS
BOOST_SYSTEM
BOOST_THREADS
BOOST_TIMER
BOOST_TEST

AM_CONDITIONAL([DARWIN],[test `uname` == Darwin])
AM_CONDITIONAL([LINUX], [test `uname` == Linux])

AC_CHECK_HEADERS([execinfo.h cxxabi.h strings.h])
AC_CHECK_TYPES([max_align_t])

AC_CHECK_FUNCS([strcasecmp strncasecmp _stricmp _strnicmp])
AC_CHECK_FUNCS([strlwr snprintf _snprintf])

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(etc/Makefile)

AC_CONFIG_FILES(src/Makefile)

AC_CONFIG_FILES(src/external/glad/Makefile)
AC_CONFIG_FILES(src/external/lua/Makefile)
AC_CONFIG_FILES(src/external/mongoose/Makefile)

AC_OUTPUT()

