#pragma once

#include "AnimationComponent.h"
#include "System.h"

namespace GPC {

    class AnimationSystem : public System {
    public:
        BIND_SYSTEM(ANIMATION);

        void OnUpdate() override;
    };

} // GPC
