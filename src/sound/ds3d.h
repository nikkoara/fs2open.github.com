// -*- mode: c++; -*-

#ifndef FREESPACE2_SOUND_DS3D_H
#define FREESPACE2_SOUND_DS3D_H

int ds3d_update_listener (vec3d* pos, vec3d* vel, matrix* orient);
int ds3d_update_buffer (
    int channel, float min, float max, vec3d* pos, vec3d* vel);

#endif /* FREESPACE2_SOUND_DS3D_H */
