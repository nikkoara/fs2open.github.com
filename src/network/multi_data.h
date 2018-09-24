// -*- mode: c++; -*-

#ifndef FREESPACE2_NETWORK_MULTI_DATA_H
#define FREESPACE2_NETWORK_MULTI_DATA_H

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
