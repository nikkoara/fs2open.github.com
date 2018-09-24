// -*- mode: c++; -*-

#ifndef FREESPACE2_SCRIPTING_API_OBJS_PARTICLE_H
#define FREESPACE2_SCRIPTING_API_OBJS_PARTICLE_H

#include "scripting/ade_api.h"
#include "particle/particle.h"

namespace scripting {
namespace api {

class particle_h {
protected:
    particle::WeakParticlePtr part;

public:
    particle_h ();

    explicit particle_h (const particle::WeakParticlePtr& part_p);

    particle::WeakParticlePtr Get ();

    bool isValid ();
};

DECLARE_ADE_OBJ (l_Particle, particle_h*);

} // namespace api
} // namespace scripting

#endif // FREESPACE2_SCRIPTING_API_OBJS_PARTICLE_H
