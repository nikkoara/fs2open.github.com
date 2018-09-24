// -*- mode: c++; -*-

#ifndef FREESPACE2_NETWORK_MULTI_UPDATE_H
#define FREESPACE2_NETWORK_MULTI_UPDATE_H

// -------------------------------------------------------------------------------------------------------------------
// MULTI UPDATE DEFINES/VARS
//

// operation return codes
#define MULTI_UPDATE_CONTINUE 0      // continue to next screen
#define MULTI_UPDATE_SHUTTING_DOWN 1 // freespace is exiting to the launcher
#define MULTI_UPDATE_MAIN_MENU 2 // caller should move back to the main menu

// -------------------------------------------------------------------------------------------------------------------
// MULTI UPDATE FUNCTIONS
//

// check to see if the version of FS on this machine is not recent. run in a
// popup if the versions don't check out, bail to the launcher see
// MULTI_UPDATE_* return codes, above
int multi_update_gobaby ();

#endif
