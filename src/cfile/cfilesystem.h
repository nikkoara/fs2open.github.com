// -*- mode: c++; -*-

#ifndef FREESPACE2_CFILE_CFILESYSTEM_H
#define FREESPACE2_CFILE_CFILESYSTEM_H

#include "cfile/cfile.h"

// Builds a list of all the files
void cf_build_secondary_filelist (const char* cdrom_path);
void cf_free_secondary_filelist ();

// Internal stuff
struct cf_pathtype  {
    int index; // To verify that the CF_TYPE define is correctly indexed into
               // this array
    const char* path; // Path relative to FreeSpace root, has ending backslash.
    const char*
        extensions;   // Extensions used in this pathtype, separated by spaces
    int parent_index; // Index of this directory's parent.  Used for creating
                      // directories when writing.
};

// During cfile_init, verify that Pathtypes[n].index == n for each item
extern cf_pathtype Pathtypes[CF_MAX_PATH_TYPES];

bool cf_check_location_flags (uint32_t check_flags, uint32_t desired_flags);

// Returns the default storage path for files given a
// particular pathtype.   In other words, the path to
// the unpacked, non-cd'd, stored on hard drive path.
// If filename isn't null it will also tack the filename
// on the end, creating a completely valid filename.
// Input:   pathtype  - CF_TYPE_??
// path_max  - Maximum characters in the path
// filename  - optional, if set, tacks the filename onto end of path.
// Output:  path      - Fully qualified pathname.
// Returns 0 if result would be too long (invalid result)
int cf_create_default_path_string (
    char* path, uint path_max, int pathtype, const char* filename = nullptr,
    bool localize = false, uint32_t location_flags = CF_LOCATION_ALL);
int cf_create_default_path_string (
    std::string& path, int pathtype, const char* filename = nullptr,
    bool localize = false, uint32_t location_flags = CF_LOCATION_ALL);

#endif //FREESPACE2_CFILE_CFILESYSTEM_H