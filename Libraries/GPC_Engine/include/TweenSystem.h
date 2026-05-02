#pragma once
#include "Component.h"
#include "Tween.h"
#include <vector>
#include <memory>

#include "System.h"

namespace GPC
{
    struct TweenComponent : public Component
    {
        BIND_COMPONENT(TWEEN)
        AUTO_COMPONENT_CONSTRUCTOR(TweenComponent)

        std::vector<std::shared_ptr<ITween>> ActiveTweens;

        template<typename T>
        void AddTween(TweenConfig<T>& tween) {
            ActiveTweens.push_back(std::make_shared<Tween<T>>(tween));
        }
    };

    class TweenSystem : public System
    {
    public:
        BIND_SYSTEM(TWEEN)

        TweenSystem() = default;
        ~TweenSystem() override = default;

        void OnUpdate() override;

    };
}
