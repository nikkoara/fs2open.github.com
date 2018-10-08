// -*- mode: c++; -*-

#ifndef FREESPACE2_HUD_HUDOBSERVER_H
#define FREESPACE2_HUD_HUDOBSERVER_H

// prototypes
class ship;
struct ai_info;

// use these to redirect Player_ship and Player_ai when switching into ai mode
extern ship Hud_obs_ship;
extern ai_info Hud_obs_ai;

void hud_observer_init (ship* shipp, ai_info* aip);

#endif
