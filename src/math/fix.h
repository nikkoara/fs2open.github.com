// -*- mode: c++; -*-

#ifndef FREESPACE2_MATH_FIX_H
#define FREESPACE2_MATH_FIX_H

#define F1_0 65536
#define f1_0 65536

fix fixmul (fix a, fix b);
fix fixdiv (fix a, fix b);
fix fixmuldiv (fix a, fix b, fix c);

constexpr int
f2i (fix a) {
    return static_cast< int > (a >> 16);
}

constexpr fix
i2f (int a) {
    return static_cast< fix > (a << 16);
}

#endif // FREESPACE2_MATH_FIX_H
