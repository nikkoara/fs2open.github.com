dnl -*- Autoconf -*-

AC_DEFUN([AC_CHECK___VA_COPY],[
  AS_VAR_PUSHDEF([ac_Var],[ac_fs2_cv_have___va_copy])
  AC_CACHE_CHECK([for __va_copy],[ac_Var],[
      AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      AC_TRY_LINK([
#include <stdarg.h>
va_list ap1,ap2;
	], [
__va_copy(ap1,ap2);
	],
        [AS_VAR_SET([ac_Var], [yes])],
        [AS_VAR_SET([ac_Var], [no])])
      AC_LANG_RESTORE
  ])
  AS_IF([test AS_VAR_GET([ac_Var]) = no], 
        [AC_DEFINE(FS2_NO__VA_COPY,1,[Missing __va_copy])], [])
  AS_VAR_POPDEF([ac_Var])dnl
])

AC_DEFUN([AC_CHECK_VA_COPY],[
  AS_VAR_PUSHDEF([ac_Var],[ac_fs2_cv_have_va_copy])
  AC_CACHE_CHECK([for va_copy],[ac_Var],[
      AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      AC_TRY_LINK([
#include <stdarg.h>
va_list ap1,ap2;
	], [
va_copy(ap1,ap2);
	],
        [AS_VAR_SET([ac_Var], [yes])],
        [AS_VAR_SET([ac_Var], [no])])
      AC_LANG_RESTORE
  ])
  AS_IF([test AS_VAR_GET([ac_Var]) = no], 
        [AC_DEFINE(FS2_NO_VA_COPY,1,[Missing va_copy])])
  AS_VAR_POPDEF([ac_Var])dnl
])
