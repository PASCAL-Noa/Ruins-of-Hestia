#pragma once
#include "LivingEntityBehavior.h"

namespace GPC {

    struct HealthBarController : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(HealthBarController);

        EnnemiGeneratorBehavior::EnnemiComponents* pComponents = nullptr;

        float BaseScale = 1.2f * GENERAL_SCALE_EXPEDITION;
        static constexpr float SCALE_Y_HEALTH = 0.05f * GENERAL_SCALE_EXPEDITION;
        static constexpr float HEIGHT = 0.7f * GENERAL_SCALE_EXPEDITION;

        void OnUpdate(const BehaviorUpdateContext *pCtx) override;
    };

} // GPC