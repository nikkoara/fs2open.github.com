// -*- mode: c++; -*-

#ifndef FREESPACE2_UTIL_FMT_H
#define FREESPACE2_UTIL_FMT_H

#include "defs.hpp"

#include <cstdarg>
#include <cstddef>

#include <string>

char* fs2_snfmt  (char*, size_t, char const*, ...);
char* fs2_vsnfmt (char*, size_t, char const*, va_list);

std::string
fs2_fmt (const char*, int, const char* = 0, ...);

#endif // FREESPACE2_UTIL_FMT_H