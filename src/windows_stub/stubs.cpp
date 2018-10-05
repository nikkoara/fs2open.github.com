// -*- mode: c++; -*-

#include "defs.hpp"

#include <cctype>
#include <cerrno>
#include <fcntl.h>
#include <cstdarg>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "windows_stub/config.h"

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifdef HAVE_CXXABI_H
#include <cxxabi.h>
#endif

#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "globalincs/pstypes.h"

// find the size of a file
int filelength (int fd) {
    struct stat buf;

    if (fstat (fd, &buf) == -1) return -1;

    return buf.st_size;
}

// get a filename minus any leading path
char* clean_filename (char* name) {
    char* p = name + strlen (name) - 1;

    // Move p to point to first letter of EXE filename
    while ((p > name) && (*p != '\\') && (*p != '/') && (*p != ':')) p--;

    p++;

    return p;
}

// retrieve the current working directory
int _getcwd (char* out_buf, unsigned int len) {
    if (getcwd (out_buf, len) == NULL) {
        ASSERTF (
            __FILE__, __LINE__, "buffer overflow in getcwd (buf size = %u)",
            len);
    }

    return 1;
}

// change directory to specified path
int _chdir (const char* path) {
    int status = chdir (path);

#ifndef NDEBUG
    int m_error = errno;

    if (status) {
        WARNINGF (
            __FILE__, __LINE__, "Cannot chdir to %s: %s", path,
            strerror (m_error));
    }
#endif

    return status;
}

// make specified directory
int _mkdir (const char* path) {
    // Windows _mkdir does not take file permissions as a parameter.
    // umask already deals with that, so 0777 should be fine.
    return mkdir (path, 0777);
}

void _splitpath (
    char* path, char* /*drive*/, char* dir, char* fname, char* ext) {
    if ((path == NULL) || (fname == NULL)) return;

    // stop at these in case they ever get used, we need to support them at
    // that point
    ASSERT ((dir == NULL) && (ext == NULL));

    /* fs2 only uses fname */
    if (fname != NULL) {
        const char* ls = strrchr (path, '/');

        if (ls != NULL) {
            ls++; // move past '/'
        }
        else {
            ls = path;
        }

        const char* lp = strrchr (path, '.');

        if (lp == NULL) {
            lp = ls + strlen (ls); // move to the end
        }

        int dist = lp - ls;

        if (dist > (_MAX_FNAME - 1)) dist = _MAX_FNAME - 1;

        strncpy (fname, ls, dist);
        fname[dist] = 0; // add null, just in case
    }
}

int MulDiv (int number, int numerator, int denominator) {
    int result;

    if (denominator == 0) return 0;

    longlong tmp;
    tmp = ((longlong)number) * ((longlong)numerator);
    tmp /= (longlong)denominator;
    result = (int)tmp;

    return result;
}
