#pragma once

#include "Behavior.h"

namespace GPC {

    struct SpawnerDebug : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(SpawnerDebug);

        void OnUpdate(const BehaviorUpdateContext *pCtx) override;
    };

} // GPC