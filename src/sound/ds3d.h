// -*- mode: c++; -*-

#ifndef __DS3D_H__
#define __DS3D_H__

int ds3d_update_listener (vec3d* pos, vec3d* vel, matrix* orient);
int ds3d_update_buffer (
    int channel, float min, float max, vec3d* pos, vec3d* vel);

#endif /* __DS3D_H__ */
