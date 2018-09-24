// -*- mode: c++; -*-

#ifndef __POPUPDEAD_H__
#define __POPUPDEAD_H__

// return values for popup_do_frame for multiplayer
#define POPUPDEAD_DO_RESPAWN 0
#define POPUPDEAD_DO_OBSERVER 1
#define POPUPDEAD_DO_MAIN_HALL 2

void popupdead_start ();
void popupdead_close ();
int popupdead_do_frame (float frametime);
int popupdead_is_active ();

#endif
