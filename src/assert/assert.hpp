// -*- mode: c++; -*-

#ifndef FREESPACE2_ASSERT_ASSERT_H
#define FREESPACE2_ASSERT_ASSERT_H

#ifndef NDEBUG
//
// Printf-like assertion:
//
void
assertf (const char*, int, const char* = 0, ...)
    __attribute__ ((format (printf, 3, 4)));

//
// Printf-like assertion with an extra prefix:
//
void
assertx (const char*, const char*, int, const char* = 0, ...)
    __attribute__ ((format (printf, 4, 5)));

#  define ASSERTX(expr, fmt, ...)                               \
    (static_cast< bool > ((expr))                               \
     ? void (0)                                                 \
     : assertx (#expr, __FILE__, __LINE__, fmt, ##__VA_ARGS__))

#  define ASSERTF assertf
#else
#  define ASSERTX(expr, ignore, ...) FS2_UNUSED ((expr)) 
#  define ASSERTF(...) ((void)0)
#endif // NDEBUG

#endif // FREESPACE2_ASSERT_ASSERT_H
