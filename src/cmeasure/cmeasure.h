// -*- mode: c++; -*-

#ifndef FREESPACE2_CMEASURE_CMEASURE_H
#define FREESPACE2_CMEASURE_CMEASURE_H

#include "globalincs/globals.h"
#include "globalincs/systemvars.h"

class object;

#define CMEASURE_WAIT \
    333 //	delay in milliseconds between countermeasure firing.

//	Maximum distance at which a countermeasure can be tracked
//	If this value is too large, missiles will always be tracking
// countermeasures.
#define MAX_CMEASURE_TRACK_DIST 300.0f
extern const float CMEASURE_DETONATE_DISTANCE;

extern int Cmeasures_homing_check;
extern int Countermeasures_enabled;

extern void
cmeasure_set_ship_launch_vel (object* objp, object* parent_objp, int arand);
extern void cmeasure_select_next (object* objp);
extern void cmeasure_maybe_alert_success (object* objp);

#endif // FREESPACE2_CMEASURE_CMEASURE_H
