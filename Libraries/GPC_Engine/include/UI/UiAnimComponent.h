#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "Component.h"
#include "Tweens.h"

namespace GPC
{
    enum class UiAnimLoop : uint8_t
    {
        None     = 0,
        Restart  = 1,
        PingPong = 2
    };

    constexpr uint32_t UiAnimHashTag(const char* s)
    {
        uint32_t h = 2166136261u;
        while (*s)
        {
            h ^= static_cast<uint32_t>(*s++);
            h *= 16777619u;
        }
        return h;
    }

    struct UiAnimSlot
    {
        float                           Elapsed       = 0.0f;
        float                           Duration      = 0.0f;
        float                           Delay         = 0.0f;
        float                           PlaybackRate  = 1.0f;
        Tweening::EasingType            Ease          = Tweening::EasingType::Linear;
        std::function<void(float)>      Setter;
        std::function<void()>           OnStart;
        std::function<void()>           OnComplete;
        std::function<void(float)>      OnUpdate;
        uint32_t                        TagHash       = 0;
        int32_t                         RepeatCount   = 0;
        UiAnimLoop                      Loop          = UiAnimLoop::None;
        bool                            Started       = false;
        bool                            Reversed      = false;
    };

    struct UiAnimComponent : public Component
    {
        static constexpr bool kStableSlot = true;

        BIND_COMPONENT(UI_ANIM);
        AUTO_COMPONENT_CONSTRUCTOR(UiAnimComponent);

        std::vector<UiAnimSlot> Active;
        float                   PlaybackRate = 1.0f;
        bool                    PlaybackActive = true;
    };
}
