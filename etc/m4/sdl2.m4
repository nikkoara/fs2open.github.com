# -*- mode: autoconf; -*-

AC_DEFUN([AC_CONFIG_WITH_SDL2],[

AC_MSG_CHECKING([for SDL2]) 

if ! pkg-config --exists sdl2 &>/dev/null; then
  echo 'missing pkg-config for SDL2'; exit 1
fi

ac_sdl2_cppflags="$( pkg-config --cflags sdl2 )"
ac_sdl2_ldflags="$( pkg-config --libs sdl2 )"

AC_SUBST(SDL2_CPPFLAGS,[$ac_sdl2_cppflags])
AC_SUBST(SDL2_LDFLAGS,[$ac_sdl2_ldflags])

AC_MSG_RESULT([done])
])
