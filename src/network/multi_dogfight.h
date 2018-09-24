// -*- mode: c++; -*-

#ifndef __FS2_MULTIPLAYER_DOGFIGHT_HEADER_FILE
#define __FS2_MULTIPLAYER_DOGFIGHT_HEADER_FILE

// ----------------------------------------------------------------------------------------------------
// MULTI DOGFIGHT DEFINES/VARS
//

struct net_player;
class object;

// ----------------------------------------------------------------------------------------------------
// MULTI DOGFIGHT FUNCTIONS
//

// call once per level just before entering the mission
void multi_df_level_pre_enter ();

// evaluate a kill in dogfight by a netplayer
void multi_df_eval_kill (net_player* killer, object* dead_obj);

// debrief
void multi_df_debrief_init ();

// do frame
void multi_df_debrief_do ();

// close
void multi_df_debrief_close ();

#endif
