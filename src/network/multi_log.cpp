// -*- mode: c++; -*-

#include <cstdarg>
#include "network/multi_log.h"
#include "cfile/cfile.h"
#include "parse/parselo.h"

// max length for a line of the logfile
#define MAX_LOGFILE_LINE_LEN 256

// how often we'll write an update to the logfile (in seconds)
#define MULTI_LOGFILE_UPDATE_TIME 2520 // every 42 minutes

// time when the logfile was opened
int Multi_log_open_systime = -1;

// time when we last updated the logfile
int Multi_log_update_systime = -1;

// write out some info about stuff
void multi_log_write_update () {
    int diff = (int)difftime (time (NULL), Multi_log_open_systime);
    int hours, mins, seconds;

    // figure out some time values
    hours = diff / 3600;
    mins = (diff - (hours * 3600)) / 60;
    seconds = (diff - (hours * 3600) - (mins * 60));

    // print it out
    ml_printf (
        "Server has been active for %d hours, %d minutes, and %d seconds",
        hours, mins, seconds);
}

// initialize the multi logfile
void multi_log_init () {
    II ("multiplayer") << "logging started";
    Multi_log_update_systime = Multi_log_open_systime = time (0);
}

// close down the multi logfile
void multi_log_close () { II ("multiplayer") << "logging closed"; }

// give some processing time to the logfile system so it can check up on stuff
void multi_log_process () {
    // check to see if we've been active a long enough time, and
    if (time (NULL) - Multi_log_update_systime > MULTI_LOGFILE_UPDATE_TIME) {
        // write the update
        multi_log_write_update ();
        Multi_log_update_systime = (int)time (NULL);
    }
}

// printf function itself called by the ml_printf macro
void ml_printf (const char* fmt, ...) {
    if (0 == fmt || 0 == fmt[0]) return;

    std::string temp;
    va_list args;

    // fmt the text
    va_start (args, fmt);
    vsprintf (temp, fmt, args);
    va_end (args);

    // log the string including the time
    II ("multiplayer") << temp.c_str ();
}

// string print function
void ml_string (const char* string, int add_time) {
    char tmp[MAX_LOGFILE_LINE_LEN * 4];
    char time_str[128];
    time_t timer;

    // if the passed string is NULL, do nothing
    if (string == NULL) { return; }

    // maybe add the time
    if (add_time) {
        timer = time (NULL);

        strftime (time_str, 128, "%m/%d %H:%M:%S~   ", localtime (&timer));
        strcpy_s (tmp, time_str);
        strcat_s (tmp, string);
    }
    else {
        strcpy_s (tmp, string);
    }
    // don't need to add terminating \n since log_string() will do it

    // now print it to the logfile if necessary
    II ("multiplayer") << tmp;

    // add to standalone UI too
    extern int Is_standalone;
    extern void std_debug_multilog_add_line (const char* str);
    if (Is_standalone) { std_debug_multilog_add_line (tmp); }

#if defined(MULTI_LOGFILE_ECHO_TO_DEBUG)
    WARNINGF (LOCATION, "ML %s", tmp);
#endif
}
