// -*- mode: c++; -*-

#ifndef _MULTI_PLAYER_DATA_HEADER_FILE
#define _MULTI_PLAYER_DATA_HEADER_FILE

// -------------------------------------------------------------------------
// MULTI DATA DEFINES/VARS
//

// -------------------------------------------------------------------------
// MULTI DATA FUNCTIONS
//

// reset the data xfer system
void multi_data_reset ();

// handle a player join (clear out lists of files, etc)
void multi_data_handle_join (int player_index);

// handle a player drop (essentially the same as multi_data_handle_join)
void multi_data_handle_drop (int player_index);

// do all sync related data stuff (server-side only)
void multi_data_do ();

// handle an incoming xfer request from the xfer system
void multi_data_handle_incoming (int handle);

// send all my files as necessary
void multi_data_send_my_junk ();

#endif
