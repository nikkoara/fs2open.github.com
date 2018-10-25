dnl -*- Autoconf -*-

AC_DEFUN([AC_CHECK_VSNPRINTF_POSIX_OVERFLOW],[
  AS_VAR_PUSHDEF([ac_Var],[ac_fs2_cv_vsnprintf_overflow_return])
  AC_CACHE_CHECK([for vsnprintf overflow POSIX compliance], [ac_Var],[
     AC_LANG_SAVE
     AC_LANG_CPLUSPLUS
     AC_RUN_IFELSE([
       AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
@%:@include <stdio.h>
@%:@include <stdarg.h>
@%:@include <stdio.h>
static int 
test_vsnprintf (char* pbuf, size_t len, char const* fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    int const n = vsnprintf (pbuf, len, fmt, ap);
    va_end (ap);

    return n;
}

],[
    char buf [[3]], data [[]] = "foobar";
    int n = test_vsnprintf (buf, sizeof buf, "%s", data);
    return (n == int (sizeof data) - 1) ? 0 : 1;
])],
       [AS_VAR_SET([ac_Var], [yes])],
       [AS_VAR_SET([ac_Var], [no])])
     AC_LANG_RESTORE])
  AS_IF([test AS_VAR_GET([ac_Var]) = yes], [],
        [AC_DEFINE(
           [FS2_NO_POSIX_VSNPRINTF], [1],
           [Define to 1 if vsnprintf overflow is not POSIX compliant.])])
  AS_VAR_POPDEF([ac_Var])
])
