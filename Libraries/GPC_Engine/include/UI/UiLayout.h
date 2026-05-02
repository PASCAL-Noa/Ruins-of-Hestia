#pragma once

#include <glm/glm.hpp>

#include "Component.h"

namespace GPC
{
    enum class UiLayoutMode : uint8_t
    {
        Stack   = 0,
        Row     = 1,
        Column  = 2,
        Grid    = 3
    };

    enum class UiJustify : uint8_t
    {
        Start   = 0,
        Center  = 1,
        End     = 2,
        Space   = 3
    };

    enum class UiAlign : uint8_t
    {
        Start   = 0,
        Center  = 1,
        End     = 2,
        Stretch = 3
    };

    struct UiLayout : public Component
    {
        static constexpr bool kStableSlot = true;

        BIND_COMPONENT(UI_LAYOUT);
        AUTO_COMPONENT_CONSTRUCTOR(UiLayout);

        UiLayoutMode Mode         = UiLayoutMode::Stack;
        UiJustify    Justify      = UiJustify::Start;
        UiAlign      AlignItems   = UiAlign::Start;
        UiAlign      AlignSelf    = UiAlign::Start;

        glm::vec4     Padding      = { 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4     Margin       = { 0.0f, 0.0f, 0.0f, 0.0f };
        float         Gap          = 0.0f;

        float         FlexGrow     = 0.0f;
        float         FlexShrink   = 1.0f;
        float         BasisWidth   = 0.0f;
        float         BasisHeight  = 0.0f;

        uint32_t      GridCols     = 1;
        uint32_t      GridRows     = 1;

        bool          IsDirty      = true;
    };
}
