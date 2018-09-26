// -*- mode: c++; -*-

#ifndef FREESPACE2_GLOBALINCS_TOOLCHAIN_CLANG_H
#define FREESPACE2_GLOBALINCS_TOOLCHAIN_CLANG_H

/*
 * @file
 *
 * @brief Macros to abstract compiler capabilities for the Clang toolchain
 *
 * @internal
 * This file should never be included directly; instead, one should
 * include toolchain.h which will pull in the file appropriate to
 * the detected toolchain.
 */

#define SCP_FORMAT_STRING
#define SCP_FORMAT_STRING_ARGS(x, y) __attribute__ ((format (printf, x, y)))

#define __UNUSED __attribute__ ((__unused__))
#define __ALIGNED(x) __attribute__ ((__aligned__ (x)))

#ifdef NO_RESTRICT_USE
#define RESTRICT
#else
#define RESTRICT restrict
#endif

#define ASSUME(x)

#if defined(NDEBUG)
#define Assertion(expr, msg, ...) \
    do {                          \
    } while (false)
#else
 /*
  * NOTE: Assertion() can only use its proper functionality in compilers
  * that support variadic macros.
  */
#define Assertion(expr, msg, ...)                               \
    do {                                                        \
        if (!(expr)) {                                          \
            os::dialogs::AssertMessage (                        \
                #expr, __FILE__, __LINE__, msg, ##__VA_ARGS__); \
        }                                                       \
    } while (false)
#endif

 /* C++11 Standard Detection */
#if !defined(HAVE_CXX11)
 /*
  * Clang does not seem to have a feature check for 'is_trivial'.
  * Assume it will be covered by one of the following checks ...
  * http://clang.llvm.org/docs/LanguageExtensions.html#feature_check
  */
#if __has_feature(cxx_static_assert) && __has_feature(cxx_auto_type)
#define HAVE_CXX11
#endif
#endif

#define SIZE_T_ARG "%zu"
#define PTRDIFF_T_ARG "%zd"

#define USED_VARIABLE __attribute__ ((used))

#ifndef CLANG_ANALYZER_NORETURN
#if __has_feature(attribute_analyzer_noreturn)
#define CLANG_ANALYZER_NORETURN __attribute__ ((analyzer_noreturn))
#else
#define CLANG_ANALYZER_NORETURN
#endif
#endif

#ifndef NDEBUG
#define UNREACHABLE(msg, ...)                                        \
    do {                                                             \
        os::dialogs::Error (__FILE__, __LINE__, msg, ##__VA_ARGS__); \
    } while (false)
#else
#define UNREACHABLE(msg, ...) __builtin_unreachable ()
#endif

#endif // FREESPACE2_GLOBALINCS_TOOLCHAIN_CLANG_H
