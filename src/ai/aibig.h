// -*- mode: c++; -*-

#ifndef __AIBIG_H__
#define __AIBIG_H__

class object;
struct ai_info;
struct vec3d;
class ship_subsys;

void ai_big_chase ();
void ai_big_subsys_path_cleanup (ai_info* aip);

// strafe functions
void ai_big_strafe ();
int ai_big_maybe_enter_strafe_mode (const object* objp, int weapon_objnum);
void ai_big_strafe_maybe_attack_turret (
    const object* ship_objp, const object* weapon_objp);
void ai_big_pick_attack_point (
    object* objp, object* attacker_objp, vec3d* attack_point,
    float fov = 1.0f);
void ai_big_pick_attack_point_turret (
    object* objp, ship_subsys* ssp, vec3d* gpos, vec3d* gvec,
    vec3d* attack_point, float weapon_travel_dist, float fov = 1.0f);

#endif
