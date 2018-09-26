dnl -*- Autoconf -*-

dnl TODO: enable all warnings

AC_DEFUN([AC_CONFIG_CXX_WARNINGS],[
WARNFLAGS=" -W -Wall \
    -Wno-parentheses \
    -Wno-deprecated-declarations \
    -Wno-empty-body \
    -Wno-char-subscripts"

CLANG_WARNFLAGS=" \
    -Wno-deprecated-register \
    -Wno-logical-op-parentheses"

GCC_WARNFLAGS=" \
    -Wno-format-overflow \
    -Wno-stringop-truncation \
    -Wno-stringop-overflow"

case $CXX in
    clang++)
        WARNFLAGS+="$CLANG_WARNFLAGS"
        ;;
    g++)
        WARNFLAGS+="$GCC_WARNFLAGS"
        ;;
     *) ;;
esac
  
CFLAGS+="$WARNFLAGS"
CXXFLAGS+="$WARNFLAGS"

])
