// -*- mode: c++; -*-

#ifndef FREESPACE2_OSAPI_DIALOGS_H
#define FREESPACE2_OSAPI_DIALOGS_H

#include "defs.hpp"
#include "globalincs/pstypes.h"

#include <stdexcept>

namespace fs2 {
namespace dialog {

enum struct dialog_type {
    error, warning, info
};

//
// Used by Assert/Assertion, don't use directly.
//
void assert_msg (const char*, const char*, int, const char* = 0, ...)
    __attribute__ ((format (printf, 4, 5)));

//
// Irrecoverable state, stop. Use Assert/Assertion for recoverable states.
//
void error (const char*, int, const char*, ...)
    __attribute__ ((format (printf, 3, 4)));

// 
// More general version of the above, only displays a dialog.
//
void error (const char*);

//
// Warnings, only enabled in debug builds.
//
void warning (const char*, int, const char*, ...)
    __attribute__ ((format (printf, 3, 4)));

//
// Debug+Release version of the above.
//
void release_warning (const char*, int, const char*, ...)
    __attribute__ ((format (printf, 3, 4)));

//
// Extra warning, only shows when command line contains `-extra_warn'
//
void warning_ex (const char*, int, const char*, ...)
    __attribute__ ((format (printf, 3, 4)));

void message (dialog_type, const char*, const char* = 0);

} // namespace dialog
} // namespace fs2

#endif // FREESPACE2_OSAPI_DIALOGS_H
