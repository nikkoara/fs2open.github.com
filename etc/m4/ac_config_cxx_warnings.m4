dnl -*- Autoconf -*-

AC_DEFUN([AC_CONFIG_CXX_WARNINGS],[

WARNFLAGS=" -W -Wall \
    -Wno-parentheses \
    -Wno-deprecated-declarations \
    -Wno-empty-body \
    -Wno-deprecated-register \
    -Wno-logical-op-parentheses"

CFLAGS+="$WARNFLAGS"
CXXFLAGS+="$WARNFLAGS"

])
