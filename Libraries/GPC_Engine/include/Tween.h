#pragma once
#include <functional>
#include <algorithm>
#include "Tweens.h"

namespace GPC
{
    class ITween
    {
    public:
        virtual                     ~ITween() = default;
        virtual void                Update(float dt) = 0;
        virtual bool                IsFinished() const = 0;
    };

    template<typename T>
    struct TweenConfig
    {
        T                           Start{};
        T                           End{};
        float                       Duration = 0.0f;
        std::function<void(T)>      Setter;
        Tweening::EasingType        Ease = Tweening::EasingType::Linear;
    };

    template<typename T>
    class Tween : public ITween
    {
    public:
        Tween(const TweenConfig<T>& config);
        void                        Update(float dt) override;
        [[nodiscard]] bool          IsFinished() const override { return m_Elapsed >= m_Config.Duration; }

    private:
        TweenConfig<T>              m_Config;
        float                       m_Elapsed = 0.0f;
        Tweening::EasingFunction    m_EaseFunc;
    };
}

#include "Tween.inl"