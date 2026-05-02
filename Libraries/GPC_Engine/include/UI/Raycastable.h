#pragma once

#include <glm/glm.hpp>

#include "Component.h"

namespace GPC
{
    enum class RaycastShape : uint8_t
    {
        Rect   = 0,
        Circle = 1,
        None   = 2
    };

    struct Raycastable : public Component
    {
        BIND_COMPONENT(UI_RAYCAST);
        AUTO_COMPONENT_CONSTRUCTOR(Raycastable);

        RaycastShape Shape      = RaycastShape::Rect;
        float        Radius     = 0.0f;
        glm::vec2    RectInset  = { 0.0f, 0.0f };
        bool         Blocking   = true;
    };
}
