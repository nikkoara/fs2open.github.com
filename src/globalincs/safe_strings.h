// -*- mode: c++; -*-

#ifndef FREESPACE2_GLOBALINCS_SAFE_STRINGS_H
#define FREESPACE2_GLOBALINCS_SAFE_STRINGS_H

/* It is a condition of use that safe_strings.cpp, safe_strings.h,
 * safe_strings_test.cpp remain together.
 *
 * Maintained by portej05 - contact via PM on www.hard-light.net/forums
 * Why have we got this, what is it for?
 * VC2005+ define some safe string functions which check buffer sizes before
 * doing anything Unfortunately, GCC and MACOS do not provide these functions,
 * therefore, we must! (if only to reduce the amount of noise the static
 * analysis tools are spitting out) They are part of ISO/IEC TR 24731 and may
 * find their way into the CRTs at some point, at which point these functions
 * must be removed from the engine. While these functions do not add a huge
 * amount of benefit for heap-allocated strings, they can protect against a
 * class of buffer overruns in stack allocated situations.
 *
 */

/* Include logic:
 * Used unless we have VC6
 */

/* errno_t, EINVAL, ERANGE, etc.. */
#include <cerrno>
#include <cstdlib> /* size_t */

/* Because errno_t is not (yet) standard, we define it here like this */
typedef int errno_t;

#if !defined(NO_SAFE_STRINGS)

/* In order to compile safe_strings_test.cpp, you must have this defined on the
 * command line */
/* #define SAFESTRINGS_TEST_APP */

/* Unlike their CRT counterparts, these do not call the invalid parameter
 * handler However, they do call this macro
 */
#ifndef SAFESTRINGS_TEST_APP

#ifndef __safe_strings_error_handler
#define __safe_strings_error_handler( val )                                \
    fs2::dialog::error (                                                   \
        file, line,                                                        \
        "%s: String error. Please Report.\nTrying to put into " SIZE_T_ARG \
        " byte buffer:\n%s",                                               \
        #val, sizeInBytes, strSource)
#endif

#else

/* For testing only */
/* For testing only */
#	define __safe_strings_error_handler( errnoVal )             \
    extern void error_handler (                                 \
        int errnoValue, const char* errnoStr, const char* file, \
        const char* function, int line);                        \
    error_handler (errnoVal, #errnoVal, __FILE__, __FUNCTION__, __LINE__);
#endif

extern errno_t scp_strcpy_s (
    const char* file, int line, char* strDest, size_t sizeInBytes,
    const char* strSource);
extern errno_t scp_strcat_s (
    const char* file, int line, char* strDest, size_t sizeInBytes,
    const char* strSource);

template< size_t size >
inline errno_t scp_strcpy_s (
    const char* file, int line, char (&strDest)[size], const char* strSource) {
    return scp_strcpy_s (file, line, strDest, size, strSource);
}

template< size_t size >
inline errno_t scp_strcat_s (
    const char* file, int line, char (&strDest)[size], const char* strSource) {
    return scp_strcat_s (file, line, strDest, size, strSource);
}

#define strcpy_s(...) scp_strcpy_s (__FILE__, __LINE__, __VA_ARGS__)
#define strcat_s(...) scp_strcat_s (__FILE__, __LINE__, __VA_ARGS__)

#else

#pragma message("safe_strings disabled")

inline errno_t
strcpy_s (char* strDest, size_t sizeInBytes, const char* strSource) {
    strcpy (strDest, strSource);
    return 0;
}

inline errno_t
strcat_s (char* strDest, size_t sizeInBytes, const char* strSource) {
    strcat (strDest, strSource);
    return 0;
}

inline errno_t strcpy_s (char* strDest, const char* strSource) {
    strcpy (strDest, strSource);
    return 0;
}

inline errno_t strcat_s (char* strDest, const char* strSource) {
    strcat (strDest, strSource);
    return 0;
}

#endif // !NO_SAFE_STRINGS

#endif // FREESPACE2_GLOBALINCS_SAFE_STRINGS_H
