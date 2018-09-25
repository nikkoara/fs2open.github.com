// -*- mode: c++; -*-

#ifndef FREESPACE2_GLOBALINCS_TOOLCHAIN_H
#define FREESPACE2_GLOBALINCS_TOOLCHAIN_H

#if defined(DOXYGEN)
#include "globalincs/toolchain/doxygen.h"
#elif defined(__clang__)
#include "globalincs/toolchain/clang.h"
#elif defined(__GNUC__)
#include "globalincs/toolchain/gcc.h"
#else
#error \
    "Unknown toolchain detected!\n"           \
		"Currently supported toolchains include:\n" \
		"\tMingW, Clang, GCC, MSVC\n"               \
		"Update toolchain.h to add support for additional toolchains.\n"
#endif

#endif /* FREESPACE2_GLOBALINCS_TOOLCHAIN_H */
