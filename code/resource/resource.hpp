/*
 * Def_Files.h
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#ifndef FREESPACE2_RESOURCE_RESOURCE_HPP
#define FREESPACE2_RESOURCE_RESOURCE_HPP

#include "globalincs/pstypes.h"

struct default_file {
    default_file (
        const char* p = 0,
        const char* f = 0,
        const void* pdata = 0,
        const size_t n = 0)
        : path_type (p), filename (f), data (pdata), size (n)
        { }

    const char* path_type;
    const char* filename;
    const void* data;
    size_t size;
};

default_file
defaults_get_file (const char*);

std::vector< default_file >
defaults_get_all ();

#endif // FREESPACE2_RESOURCE_RESOURCE_HPP
