// -*- mode: c++; -*-

#ifndef SINGLE_PARTICLE_EFFECT_H
#define SINGLE_PARTICLE_EFFECT_H

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "particle/util/ParticleProperties.h"
#include "particle/util/EffectTiming.h"

namespace particle {
namespace effects {
/**
 * @ingroup particleEffects
 */
class SingleParticleEffect : public ParticleEffect {
private:
    util::ParticleProperties m_particleProperties;

    util::EffectTiming m_timing;

public:
    explicit SingleParticleEffect (const std::string& name);

    bool processSource (const ParticleSource* source) override;

    void parseValues (bool nocreate) override;

    void pageIn () override;

    void initializeSource (ParticleSource& source) override;

    EffectType getType () const override { return EffectType::Single; }

    util::ParticleProperties& getProperties () { return m_particleProperties; }

    static SingleParticleEffect* createInstance (
        int effectID, float minSize, float maxSize, float lifetime = -1.0f);
};
} // namespace effects
} // namespace particle

#endif // SINGLE_PARTICLE_EFFECT_H
