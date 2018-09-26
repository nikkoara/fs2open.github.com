# -*- mode: autoconf; -*-

AC_DEFUN([AC_CONFIG_FFMPEG_COMPONENT],[
AC_MSG_CHECKING([for $1]) 

if ! pkg-config --exists $1 &>/dev/null; then
  echo 'missing pkg-config for $1'; exit 1
fi

ac_ffmpeg_$1_cppflags="$( pkg-config --cflags $1 )"
ac_ffmpeg_$1_ldflags="$( pkg-config --libs $1 )"

AC_SUBST(AS_TR_CPP([FFMPEG_$1_CPPFLAGS]), $ac_ffmpeg_$1_cppflags)
AC_SUBST(AS_TR_CPP([FFMPEG_$1_LDFLAGS]), $ac_ffmpeg_$1_ldflags)

AC_MSG_RESULT([done])
])

AC_DEFUN([AC_CONFIG_WITH_FFMPEG],[
AC_CONFIG_FFMPEG_COMPONENT([libavcodec])
AC_CONFIG_FFMPEG_COMPONENT([libavdevice])
AC_CONFIG_FFMPEG_COMPONENT([libavformat])
AC_CONFIG_FFMPEG_COMPONENT([libavfilter])
AC_CONFIG_FFMPEG_COMPONENT([libavutil])
AC_CONFIG_FFMPEG_COMPONENT([libpostproc])
AC_CONFIG_FFMPEG_COMPONENT([libswscale])
AC_CONFIG_FFMPEG_COMPONENT([libswresample])
dnl FFMPEG_COMPONENTS="lavcodec lavdevice lavformat lavfilter lavutil lpostproc lswscale lswresample"
dnl for x in $FFMPEG_COMPONENTS; do
dnl     AC_CONFIG_FFMPEG_COMPONENT($x)
dnl done
])
