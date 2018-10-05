// -*- mode: c++; -*-

#ifndef NDEBUG

#include "defs.hpp"
#include "log/log.hpp"
#include "util/fmt.hpp"

#include <cstdarg>
#include <cstddef>

void
assertx (const char* prefix, const char* file, int line, const char* fmt, ...) {
    va_list ap;

    va_start (ap, fmt);
    auto suffix = fs2_fmt (file, line, fmt, ap);
    va_end (ap);

    EE ("general") << prefix << " : " << suffix;
    
    std::terminate ();
}

void
assertf (const char* file, int line, const char* fmt, ...) {
    va_list ap;

    va_start (ap, fmt);
    auto suffix = fs2_fmt (file, line, fmt, ap);
    va_end (ap);

    EE ("general") << suffix;

    std::terminate ();
}

#endif // NDEBUG
