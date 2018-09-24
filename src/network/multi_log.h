// -*- mode: c++; -*-

#ifndef FREESPACE2_NETWORK_MULTI_LOG_H
#define FREESPACE2_NETWORK_MULTI_LOG_H

#include "globalincs/pstypes.h"

// ----------------------------------------------------------------------------------------------------
// MULTI LOGFILE DEFINES/VARS
//

// ----------------------------------------------------------------------------------------------------
// MULTI LOGFILE FUNCTIONS
//

// initialize the multi logfile
void multi_log_init ();

// close down the multi logfile
void multi_log_close ();

// give some processing time to the logfile system so it can check up on stuff
void multi_log_process ();

// printf function itself called by the ml_printf macro
void ml_printf (SCP_FORMAT_STRING const char* format, ...)
    SCP_FORMAT_STRING_ARGS (1, 2);

// string print function
void ml_string (const char* string, int add_time = 1);

#endif
