// -*- mode: c++; -*-

#ifndef FREESPACE2_FS2NETD_TCP_SOCKET_H
#define FREESPACE2_FS2NETD_TCP_SOCKET_H

//
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
//

#include "globalincs/pstypes.h" // make sure _cdecl is defined correctly on *nix

int FS2NetD_ConnectToServer (const char* host, const char* port);
void FS2NetD_Disconnect ();

int FS2NetD_GetData (char* buffer, int blen);
int FS2NetD_SendData (char* buffer, int blen);
bool FS2NetD_DataReady ();

#endif // FREESPACE2_FS2NETD_TCP_SOCKET_H
