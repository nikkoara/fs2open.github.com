// -*- mode: c++; -*-

#ifndef FREESPACE2_STATS_STATS_H
#define FREESPACE2_STATS_STATS_H

#define MISSION_STATS 0
#define ALL_TIME_STATS 1

void show_stats_init ();

void show_stats_numbers (
    int stage, int sx, int sy, int dy = 10, int add_mission = 0);

void show_stats_label (int stage, int sx, int sy, int dy = 10);

#endif
