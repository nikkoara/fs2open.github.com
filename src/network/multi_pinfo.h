// -*- mode: c++; -*-

#ifndef _MULTI_PLAYER_INFO_HEADER_FILE
#define _MULTI_PLAYER_INFO_HEADER_FILE

// ---------------------------------------------------------------------------------------
// MULTI PLAYER INFO DEFINES/VARS
//

// prototypes
struct net_player;

// ---------------------------------------------------------------------------------------
// MULTI PLAYER INFO FUNCTIONS
//

// fire up the player info popup
void multi_pinfo_popup (net_player* np);

// is the pilot info popup currently active?
int multi_pinfo_popup_active ();

// kill the currently active popup (if any)
void multi_pinfo_popup_kill ();

// notify the popup that a player has left
void multi_pinfo_notify_drop (net_player* np);

#endif
