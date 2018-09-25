// -*- mode: c++; -*-

#ifndef FREESPACE2_DEFS_HPP
#define FREESPACE2_DEFS_HPP

#include "config.hpp"

#define BOOST_LOG_DYN_LINK 1

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

#define FS2_UNUSED(x) ((void*)x)

#endif // FREESPACE2_DEFS_HPP
