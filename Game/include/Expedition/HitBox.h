#pragma once

#include "Behavior.h"
#include "LivingEntityBehavior.h"

namespace GPC {

    struct HitBoxDescriptor {
        LivingEntityBehavior* pSourceLiving;
        bool IsMelee;
        bool TargetPlayer;
        bool DestroyOnHit;
        float Duration;
        float StartTime;
    };

    struct HitBox : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(HitBox)

        HitBoxDescriptor Descriptor;
        bool HasHit = false;

        std::set<EntityID> EnnemiHit;

        void OnUpdate(const BehaviorUpdateContext *pCtx) override;
        void OnCollision3D(const BehaviorCollision3DContext *pCtx) override;
    };

} // GPC