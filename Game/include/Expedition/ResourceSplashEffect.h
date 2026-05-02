#pragma once
#include "Behavior.h"
#include "ParticleSystem.h"

namespace GPC {

    struct ResourceSplashEffect : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(ResourceSplashEffect);

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        glm::vec3 WorldPosition;
        std::string ComputeName;

    private:
        static constexpr float SPLASH_DURATION = 1.0f;

        ParticleEmitter* mp_Emitter;
        Transform3D* mp_Transform;
        float m_TimeShouldSucide = 0.0f;

    };

} // GPC