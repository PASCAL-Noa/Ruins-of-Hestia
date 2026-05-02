#pragma once

#include "Behavior.h"

namespace GPC {

    struct CollisionTestDebug : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(CollisionTestDebug);

        void OnCollision3D(const BehaviorCollision3DContext *pCtx) override;

    };

} // GPC