#pragma once
#include "Component.h"
#include "ECS_Defines.h"

namespace GPC
{
    struct AudioListener : public Component
    {
        BIND_COMPONENT(AUDIOLISTENER);
        AUTO_COMPONENT_CONSTRUCTOR(AudioListener);
    };
}