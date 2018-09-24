// -*- mode: c++; -*-

#include "globalincs/pstypes.h"
#include "math/fix.h"

fix fixmul (fix a, fix b) {
    longlong tmp;
    tmp = (longlong)a * (longlong)b;
    return (fix) (tmp >> 16);
}

fix fixdiv (fix a, fix b) { return MulDiv (a, 65536, b); }

fix fixmuldiv (fix a, fix b, fix c) { return MulDiv (a, b, c); }
