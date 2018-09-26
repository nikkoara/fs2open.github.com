# -*- mode: autoconf; -*-

AC_DEFUN([AC_CONFIG_WITH_OPENAL],[

AC_MSG_CHECKING([for OpenAL]) 

if ! pkg-config --exists openal &>/dev/null; then
  echo 'missing pkg-config for OPENAL'; exit 1
fi

ac_openal_cppflags="$( pkg-config --cflags openal )"
ac_openal_libs="$( pkg-config --libs openal )"

AC_SUBST(OPENAL_CPPFLAGS,[$ac_openal_cppflags])
AC_SUBST(OPENAL_LIBS,[$ac_openal_libs])

AC_MSG_RESULT([done])
])
