#pragma once

#include "ECS_Defines.h"
#include "Component.h"
#include "System.h"

namespace GPC {

    struct LightingComponent : public Component {
        BIND_COMPONENT(LIGHT);
        AUTO_COMPONENT_CONSTRUCTOR(LightingComponent);
        Light* pLightData = nullptr;
    };

    class LightingSystem : public System {
    public:
        BIND_SYSTEM(LIGHT);
        void OnUpdate() override;
    };

} // GPC