# -*- mode: autoconf -*-

AC_INIT([freespace2],[3.8.1],)

AM_INIT_AUTOMAKE([-Wno-portability subdir-objects dist-bzip2 nostdinc foreign])
AC_CONFIG_HEADERS([src/_config.hpp:src/config.ac])

AC_CONFIG_MACRO_DIRS([etc/m4])

AC_PROG_CXX
AC_CONFIG_CXX_WARNINGS

AC_ENABLE_CXX_DIALECT([c++1z])
dnl AC_ENABLE_OPENMP

AC_PROG_LIBTOOL
AC_PROG_MAKE_SET(gmake)
AM_PROG_AS

AC_PKG_CONFIG_WITH([libavcodec])
AC_PKG_CONFIG_WITH([libavdevice])
AC_PKG_CONFIG_WITH([libavfilter])
AC_PKG_CONFIG_WITH([libavformat])
AC_PKG_CONFIG_WITH([libavutil])
AC_PKG_CONFIG_WITH([libpostproc])
AC_PKG_CONFIG_WITH([libswresample])
AC_PKG_CONFIG_WITH([libswscale])
AC_PKG_CONFIG_WITH([jsoncpp])
AC_PKG_CONFIG_WITH([openal])
AC_PKG_CONFIG_WITH([sdl2])
AC_PKG_CONFIG_WITH([libjpeg])
AC_PKG_CONFIG_WITH([libpng])

AC_CONFIG_WITH_RANGE3

BOOST_REQUIRE([1.66.0])

BOOST_LOG
BOOST_LOG_SETUP
BOOST_PROGRAM_OPTIONS
BOOST_SYSTEM
BOOST_THREADS

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

