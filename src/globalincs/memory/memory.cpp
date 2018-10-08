// -*- mode: c++; -*-

#include "globalincs/pstypes.h"

namespace memory {
const quiet_alloc_t quiet_alloc;
void out_of_memory () {
    ERRORF (LOCATION, "out of memory");
    ASSERTX (0, "Out of memory.  Try closing down other applications, increasing your\nvirtual memory size, or installing more physical RAM.\n");
}
} // namespace memory
