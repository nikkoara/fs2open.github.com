// -*- mode: c++; -*-

#ifndef FREESPACE2_DEFS_HPP
#define FREESPACE2_DEFS_HPP

#define BOOST_LOG_DYN_LINK 1

#include "config.hpp"
#include "assert/assert.hpp"
#include "log/log.hpp"

#ifdef NDEBUG
#  define BOOST_DISABLE_ASSERTS
#endif // NDEBUG

#include <boost/assert.hpp>
#define ASSERT BOOST_ASSERT

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

#if !defined (FS2_NO_VA_COPY)
#  define FS2_VA_COPY(a,b) va_copy (a,b)
#elif !defined (FS2_NO__VA_COPY)
#  define FS2_VA_COPY(a,b) __va_copy (a,b)
#else
#  define FS2_VA_COPY(a,b) (a) = (b)
#endif // HAVE_*VA_COPY

#if defined (FS2_NO_POSIX_VSNPRINTF)
#  define FS2_VSNFMT_MAX  (1024U * 1024U)
#else
#  define FS2_VSNFMT_MAX  (size_t(-1) - 1)
#endif // FS2_VSNPRINTF_OVERFLOW_NEGATIVE

#define LOCATION __FILE__, __LINE__

#endif // FREESPACE2_DEFS_HPP
