// -*- mode: c++; -*-

#ifndef FREESPACE2_DEFS_HPP
#define FREESPACE2_DEFS_HPP

#define BOOST_LOG_DYN_LINK 1

#include "config.hpp"

#include "log/log.hpp"

#define FS_VERSION_MAJOR 3
#define FS_VERSION_MINOR 8
#define FS_VERSION_BUILD 1

#define FS_VERSION_HAS_REVISION 0

#define FS_VERSION_REVISION 0
#define FS_VERSION_REVISION_STRING "0"

#define FS_VERSION_FULL "3.8.1"
#define FS_PRODUCT_VERSION "3.8.1"

#define FS2_DO_PASTE(a, b) a##b
#define FS2_PASTE(a, b) FS2_DO_PASTE (a, b)

#define FS2_UNUSED(x) ((void)x)

#ifdef NDEBUG
#  define BOOST_DISABLE_ASSERTS
#endif // NDEBUG

#if defined(NDEBUG)
//
// Both Assert and Assertion are hard errors:
//
#  define Assert(expr)                                              \
    do {                                                            \
        if (!(expr)) {                                              \
            fs2::dialog::assert_msg (#expr, __FILE__, __LINE__);    \
        }                                                           \
    } while (0)

#  define Assertion(expr, msg, ...)                             \
    do {                                                        \
        if (!(expr)) {                                          \
            fs2::dialog::assert_msg (                           \
                #expr, __FILE__, __LINE__, msg, ##__VA_ARGS__); \
        }                                                       \
    } while (0)

//
// Unreachable is a hard error:
//
#  define UNREACHABLE(msg, ...)                                         \
    do {                                                                \
        fs2::dialog::error (__FILE__, __LINE__, msg, ##__VA_ARGS__);    \
    } while (0)
#else
#  define Assert(expr) do { FS2_UNUSED((expr)); } while (0)
#  define Assertion(expr, msg, ...) do { FS2_UNUSED((expr)); } while (0)
#  define UNREACHABLE(msg, ...) __builtin_unreachable ()
#endif

#define LOCATION __FILE__, __LINE__

//
// Both verifications are hard errors:
//
#define Verify(x)                                                       \
    do {                                                                \
        if (!(x)) {                                                     \
            fs2::dialog::error (LOCATION, "Verify failure: %s\n", #x); } \
    } while (0)

#define VerifyEx(x, y, ...)                                                 \
    do {                                                                    \
        if (!(x)) {                                                         \
            fs2::dialog::error (                                            \
                LOCATION, "Verify failure: %s with help text " #y "\n", #x, \
                ##__VA_ARGS__);                                             \
        }                                                                   \
    } while (0)

#endif // FREESPACE2_DEFS_HPP
