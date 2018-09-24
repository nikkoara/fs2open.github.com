// -*- mode: c++; -*-

#ifndef FREESPACE2_UTILS_STRINGS_H
#define FREESPACE2_UTILS_STRINGS_H

#include <ctype.h>

inline void
stolower (char* s) {
    if (s && s [0]) {
        for (; s [0]; ++s) {
            s [0] = tolower (s [0]);
        }
    }
}

#endif // FREESPACE2_UTILS_STRINGS_H
