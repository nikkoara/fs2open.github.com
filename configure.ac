# -*- mode: autoconf -*-

AC_INIT([freespace2],[3.8.1],)

AC_CONFIG_MACRO_DIR([etc/m4])
AC_CONFIG_AUX_DIR([etc/m4])

AM_INIT_AUTOMAKE([-Wno-portability subdir-objects dist-bzip2 nostdinc foreign])
AC_CONFIG_HEADERS([code/_config.hpp:code/config.ac])

AC_PROG_OBJC
AC_PROG_CXX
AC_CONFIG_CXX_WARNINGS

AC_ENABLE_CXX_DIALECT([c++1z])
dnl AC_ENABLE_OPENMP

AC_PROG_LIBTOOL
AC_PROG_MAKE_SET(gmake)
AM_PROG_AS

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

dnl AC_CONFIG_FILES(src/lib/jansson/Makefile)
dnl AC_CONFIG_FILES(src/lib/libjpeg/Makefile)
dnl AC_CONFIG_FILES(src/lib/libpng/Makefile)
dnl AC_CONFIG_FILES(src/lib/libsdl/Makefile)
dnl AC_CONFIG_FILES(src/lib/lua/Makefile)
dnl AC_CONFIG_FILES(src/lib/md5/Makefile)
dnl AC_CONFIG_FILES(src/lib/mongoose/Makefile)
dnl AC_CONFIG_FILES(src/lib/opengl/Makefile)
dnl AC_CONFIG_FILES(src/lib/utfcpp/Makefile)
dnl AC_CONFIG_FILES(src/lib/zlib/Makefile)

AC_CONFIG_FILES(code/Makefile)
AC_CONFIG_FILES(freespace2/Makefile)

AC_OUTPUT()

