// -*- mode: c++; -*-

#ifndef FREESPACE2_PARSE_GENERIC_LOG_H
#define FREESPACE2_PARSE_GENERIC_LOG_H

#define LOGFILE_MULTI_LOG 0
#define LOGFILE_EVENT_LOG 1

// initialize the multi logfile
bool logfile_init (int logfile_type);

// close down the multi logfile
void logfile_close (int logfile_type);

// printf function itself called by the log_printf macro
void log_printf (int logfile_type, SCP_FORMAT_STRING const char* format, ...)
    SCP_FORMAT_STRING_ARGS (2, 3);

// string print function
void log_string (int logfile_type, const char* string, int add_time = 0);

#endif
