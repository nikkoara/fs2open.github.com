// -*- mode: c++; -*-

#ifndef COMPOSIE_EFFECT_H
#define COMPOSIE_EFFECT_H

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "util/RandomRange.h"

namespace particle {
namespace effects {
/**
 * @ingroup particleEffects
 */
class CompositeEffect : public ParticleEffect {
private:
    std::vector< ParticleEffectPtr > m_childEffects;

public:
    explicit CompositeEffect (const std::string& name);

    bool processSource (const ParticleSource* source) override;

    void parseValues (bool nocreate) override;

    void pageIn () override;

    EffectType getType () const override { return EffectType::Composite; }

    const std::vector< ParticleEffectPtr >& getEffects () const {
        return m_childEffects;
    }

    void addEffect (ParticleEffectPtr effect);
};
} // namespace effects
} // namespace particle

#endif // COMPOSIE_EFFECT_H