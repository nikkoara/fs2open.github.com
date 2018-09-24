// -*- mode: c++; -*-

#ifndef _READYROOM_H
#define _READYROOM_H

extern int Sim_room_overlay_id;
extern int Campaign_room_overlay_id;

void sim_room_init ();
void sim_room_close ();
void sim_room_do_frame (float frametime);

// called by main menu to continue on with current campaign (if there is one).
int readyroom_continue_campaign ();

void campaign_room_init ();
void campaign_room_close ();
void campaign_room_do_frame (float frametime);

#endif
