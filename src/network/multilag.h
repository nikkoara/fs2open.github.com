// -*- mode: c++; -*-

#ifndef _MULTI_LAG_HEADER_FILE
#define _MULTI_LAG_HEADER_FILE

#ifndef NDEBUG
// #define MULTI_USE_LAG
#endif

#include "globalincs/pstypes.h"

struct timeval;

// initialize multiplayer lagloss. in non-debug situations, this call does
// nothing
void multi_lag_init ();

// shutdown multiplayer lag
void multi_lag_close ();

// select for multi_lag
int multi_lag_select (
    int nfds, fd_set* readfds, fd_set* writefds, fd_set* except_fds,
    timeval* timeout);

// recvfrom for multilag
int multi_lag_recvfrom (
    uint s, char* buf, int len, int flags, struct sockaddr* from,
    int* fromlen);

#endif
