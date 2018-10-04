// -*- mode: c++; -*-

#ifndef FREESPACE2_GLOBALINCS_PSTYPES_H
#define FREESPACE2_GLOBALINCS_PSTYPES_H

#include "defs.hpp"

#include "windows_stub/config.h"
#include "globalincs/scp_defines.h"
#include "utils/strings.h"

#include <cstdio> // For NULL, etc
#include <cstdlib>
#include <memory.h>
#include <cstring>
#include <algorithm>
#include <cstdint>

// value to represent an uninitialized state in any int or uint
#define UNINITIALIZED 0x7f8e6d9c

#define MAX_PLAYERS 12

#ifdef LOCAL
#undef LOCAL
#endif

#define LOCAL static // make module local varilable static.

#define DIR_SEPARATOR_CHAR '/'
#define DIR_SEPARATOR_STR "/"

typedef std::int32_t _fs_time_t; // time_t here is 64-bit and we need 32-bit
typedef std::int32_t fix;

// PTR compatible sizes
typedef ptrdiff_t ptr_s;
typedef size_t ptr_u;

typedef std::int64_t longlong;
typedef std::uint64_t ulonglong;
typedef std::uint8_t ubyte;
typedef std::uint16_t ushort;
typedef std::uint32_t uint;
typedef unsigned long ulong;

// Stucture to store clipping codes in a word
struct ccodes  {
    ubyte cc_or, cc_and; // or is low byte, and is high byte
};

struct vertex;

struct vec4  {
    union {
        struct {
            float x, y, z, w;
        } xyzw;
        float a1d[4];
    };
};

struct bvec4 {
    bool x;
    bool y;
    bool z;
    bool w;
};

// sometimes, you just need some integers
struct ivec3  {
    int x, y, z;
};

struct ivec2 {
    int x, y;
};

/** Represents a point in 3d space.

Note: this is a struct, not a class, so no member functions. */
struct vec3d  {
    union {
        struct {
            float x, y, z;
        } xyz;
        float a1d[3];
    };
};

struct vec2d  {
    float x, y;
};

struct angles_t  {
    float p, b, h;
};

struct matrix  {
    union {
        struct {
            vec3d rvec, uvec, fvec;
        } vec;
        float a2d[3][3];
        float a1d[9];
    };
};

struct matrix4  {
    union {
        struct {
            vec4 rvec, uvec, fvec, pos;
        } vec;
        float a2d[4][4];
        float a1d[16];
    };
};

struct uv_pair  {
    float u, v;
};

/** Compares two uv_pairs */
inline bool operator== (const uv_pair& left, const uv_pair& right) {
    return (left.u == right.u) && (left.v == right.v);
}

/** Represents a point in 3d screen space. 'w' is 1/z.

Like vec3d but for screens.

Note: this is a struct, not a class, so no member functions. */
struct screen3d  {
    union {
        struct {
            float x, y, w;
        } xyw;
        float a1d[3];
    };
};

/** Compares two screen3ds */
inline bool operator== (const screen3d& self, const screen3d& other) {
    return (
        self.xyw.x == other.xyw.x && self.xyw.y == other.xyw.y &&
        self.xyw.w == other.xyw.w);
}

/** Used to store rotated points for mines. Has flag to indicate if projected.

Note: this is a struct, not a class, so no memeber functions. */
struct vertex  {
    vec3d world;              // world space position
    screen3d screen;          // screen space position (sw == 1/z)
    uv_pair texture_position; // texture position
    ubyte r, g, b, a;         // color.  Use b for darkening;
    ubyte codes; // what sides of view pyramid this point is on/off.  0 =
                 // Inside view pyramid.
    ubyte flags; // Projection flags.  Indicates whether it is projected or not
                 // or if projection overflowed.
    ubyte pad[2]; // pad structure to be 4 byte aligned.
};

struct effect_vertex  {
    vec3d position;
    uv_pair tex_coord;
    float radius;
    ubyte r, g, b, a;
};

struct particle_pnt {
    vec3d position;
    float size;
    vec3d up;
};

// def_list
struct flag_def_list {
    const char* name;
    int def;
    ubyte var;
};

template< class T >
struct flag_def_list_new {
    const char* name; // The parseable representation of this flag
    T def;            // The flag definition for this flag
    bool in_use; // Whether or not this flag is currently in use or obsolete
    bool is_special; // Whether this flag requires special processing. See
                     // parse_string_flag_list<T, T> for details
};

// weapon count list (mainly for pilot files)
struct wep_t  {
    int index;
    int count;
};

struct coord2d  {
    int x, y;
};

#include "osapi/dialogs.h"

extern int Global_warning_count;
extern int Global_error_count;

#include "osapi/outwnd.h"

// To debug printf do this:
// mprintf(( "Error opening %s\n", filename ));
#ifndef NDEBUG
#define mprintf(args) outwnd_printf2 args
#define nprintf(args) outwnd_printf args
#else
#define mprintf(args)
#define nprintf(args)
#endif

#if defined(NDEBUG)
#define Int3() do { } while (0)
#else
void debug_int3 (const char* file, int line);
#define Int3() debug_int3 (__FILE__, __LINE__)
#endif // NDEBUG

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define PI 3.141592654f
// twice values
const float PI2 = (PI * 2.0f);
// half values
const float PI_2 = (PI / 2.0f);
const int RAND_MAX_2 = (RAND_MAX / 2);
const float RAND_MAX_1f = (1.0f / RAND_MAX);

extern int Fred_running;
extern bool running_unittests;

const size_t INVALID_SIZE = static_cast< size_t > (-1);

#include "math/fix.h"
#include "math/floating.h"

// Some constants for stuff
#define MAX_FILENAME_LEN 32 // Length for filenames, ie "title.pcx"
#define MAX_PATH_LEN 256    // Length for pathnames, ie "c:\bitmaps\title.pcx"

#define TRUE 1
#define FALSE 0

int myrand ();
int rand32 (); // returns a random number between 0 and 0x7fffffff

// lod checker for (modular) table parsing
struct lod_checker  {
    char filename[MAX_FILENAME_LEN];
    int num_lods;
    int override;
};

// Callback Loading function.
// If you pass a function to this, that function will get called
// around 10x per second, so you can update the screen.
// Pass NULL to turn it off.
// Call this with the name of a function.  That function will
// then get called around 10x per second.  The callback function
// gets passed a 'count' which is how many times game_busy has
// been called since the callback was set.   It gets called
// one last time with count=-1 when you turn off the callback
// by calling game_busy_callback(NULL).   Game_busy_callback
// returns the current count, so you can tell how many times
// game_busy got called.
// If delta_step is above 0, then it will also make sure it
// calls the callback each time count steps 'delta_step' even
// if 1/10th of a second hasn't elapsed.
extern int
game_busy_callback (void (*callback) (int count), int delta_step = -1);

// Call whenever loading to display cursor
extern void game_busy (const char* filename = NULL);

#define NOX(s) s

const char* XSTR (const char* str, int index);

// Caps V between MN and MX.
template< class T >
void CAP (T& v, T mn, T mx) {
    if (v < mn) { v = mn; }
    else if (v > mx) {
        v = mx;
    }
}

// faster version of CAP()
#define CLAMP(x, min, max)    \
    do {                      \
        if ((x) < (min))      \
            (x) = (min);      \
        else if ((x) > (max)) \
            (x) = (max);      \
    } while (false)

// Memory management functions
#include "globalincs/fsmemory.h"

class camid {
private:
    int sig;
    size_t idx;

public:
    camid ();
    camid (size_t n_idx, int n_sig);

    class camera* getCamera ();
    size_t getIndex ();
    int getSignature ();
    bool isValid ();
};

#include "globalincs/vmallocator.h"
#include "globalincs/safe_strings.h"

// check to see that a passed sting is valid, ie:
//  - has >0 length
//  - is not "none"
//  - is not "<none>"
inline bool VALID_FNAME (const char* x) {
    return strlen ((x)) && strcasecmp ((x), "none") != 0 &&
           strcasecmp ((x), "<none>") != 0;
}
/**
 * @brief Checks if the specified string may be a valid file name
 *
 * @warning This only does a quick check against an empty string and a few
 * known invalid names. It does not check if the file actually exists.
 *
 * @param x The file name to check
 * @return @c true if the name is valid, @c false otherwise
 */
inline bool VALID_FNAME (const std::string& x) {
    if (x.empty ()) { return false; }
    if (!strcasecmp (x.c_str (), "none")) { return false; }
    if (!strcasecmp (x.c_str (), "<none>")) { return false; }
    return true;
}

// Function to generate a stacktrace
std::string dump_stacktrace ();

#endif // FREESPACE2_GLOBALINCS_PSTYPES_H
