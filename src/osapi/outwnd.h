// -*- mode: c++; -*-

#ifndef FREESPACE2_OSAPI_OUTWND_H
#define FREESPACE2_OSAPI_OUTWND_H

#include "globalincs/pstypes.h"

#ifndef NDEBUG

void load_filter_info (void);
void outwnd_init ();
void outwnd_close ();
void outwnd_printf (const char* id, const char* format, ...)
    __attribute__ ((format (printf, 2, 3)));
void outwnd_printf2 (const char* format, ...)
    __attribute__ ((format (printf, 1, 2)));

void outwnd_debug_window_init ();
void outwnd_debug_window_do_frame (float frametime);
void outwnd_debug_window_deinit ();

extern int Log_debug_output_to_file;

#endif // NDEBUG

#endif // FREESPACE2_OSAPI_OUTWND_H
