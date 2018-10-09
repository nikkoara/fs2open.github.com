// -*- mode: c++; -*-

#ifndef FREESPACE2_MATH_FLOATING_HH
#define FREESPACE2_MATH_FLOATING_HH

#include "defs.hh"

#include <cmath>
#include <cmath>
#include <cfloat>

#include <cfloat>

extern float frand ();
extern int rand_chance (float frametime, float chance = 1.0f);
float frand_range (float min, float max);

// determine if a floating point number is NaN (Not a Number)
inline bool fl_is_nan (float fl) { return std::isnan (fl); }

// Handy macros to prevent type casting all over the place

#define fl_sqrt(fl) sqrtf (fl)
#define fl_isqrt(fl) (1.0f / sqrtf (fl))
#define fl_abs(fl) fabsf (fl)
#define i2fl(i) ((float)(i))
#define fl2i(fl) ((int)(fl))
#define fl2ir(fl) ((int)(fl + ((fl < 0.0f) ? -0.5f : 0.5f)))
#define flceil(fl) (int)ceil (fl)
#define flfloor(fl) (int)floor (fl)
#define f2fl(fx) ((float)(fx) / 65536.0f)
#define f2d(fx) (((double)(fx)) / 65536.0)
#define fl2f(fl) (int)((fl)*65536.0f)
#define fl_tan(fl) tanf (fl)

// convert a measurement in degrees to radians
#define fl_radians(fl) ((float)((fl) * (PI / 180.0f)))

// convert a measurement in radians to degrees
#define fl_degrees(fl) ((float)((fl) * (180.0f / PI)))

// sees if two floating point numbers are within the minimum tolerance
inline bool fl_equal (float a, float b) {
    return fl_abs (a - b) <=
           FLT_EPSILON * MAX (1.0f, MAX (fl_abs (a), fl_abs (b)));
}

// rounds off a floating point number to a multiple of some number
extern float fl_roundoff (float x, int multiple);

/**
 * @brief Determines if @a x falls between +/- @a e
 *
 * @param x Value to test
 * @param e Value to test against (must be positive)
 */
#define IS_NEAR_ZERO(x, e) (fl_abs (x) < (float)(e))

#endif // FREESPACE2_MATH_FLOATING_HH
