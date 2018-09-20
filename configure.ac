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

dnl BOOST_REQUIRE([1.66.0])

dnl BOOST_SMART_PTR
dnl BOOST_CHRONO
dnl BOOST_DATE_TIME
dnl BOOST_FILESYSTEM
dnl BOOST_LOG
dnl BOOST_PROGRAM_OPTIONS
dnl BOOST_SYSTEM
dnl BOOST_THREADS
dnl BOOST_TIMER
dnl BOOST_TEST

AM_CONDITIONAL([DARWIN],[test `uname` == Darwin])
AM_CONDITIONAL([LINUX], [test `uname` == Linux])

AC_CHECK_HEADERS([execinfo.h cxxabi.h strings.h])
AC_CHECK_TYPES([max_align_t])

AC_CHECK_FUNCS([strcasecmp strncasecmp _stricmp _strnicmp])
AC_CHECK_FUNCS([strlwr snprintf _snprintf])

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(etc/Makefile)

dnl AC_CONFIG_FILES(lib/jansson/Makefile)
dnl AC_CONFIG_FILES(lib/libjpeg/Makefile)
dnl AC_CONFIG_FILES(lib/libpng/Makefile)
dnl AC_CONFIG_FILES(lib/libsdl/Makefile)
dnl AC_CONFIG_FILES(lib/lua/Makefile)
dnl AC_CONFIG_FILES(lib/md5/Makefile)
dnl AC_CONFIG_FILES(lib/mongoose/Makefile)
dnl AC_CONFIG_FILES(lib/opengl/Makefile)
dnl AC_CONFIG_FILES(lib/utfcpp/Makefile)
dnl AC_CONFIG_FILES(lib/zlib/Makefile)

AC_CONFIG_FILES(code/Makefile)
AC_CONFIG_FILES(freespace2/Makefile)

AC_OUTPUT()

