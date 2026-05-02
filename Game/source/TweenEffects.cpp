#include "TweenEffects.h"
#include <cstdlib>
#include <glm/gtc/constants.hpp>

namespace GPC::TweenEffects
{
    void Shake(TweenComponent* tweenComp, Transform3D* transform, float duration, float intensity)
    {
        auto currentOffset = std::make_shared<glm::vec3>(0.0f);

        TweenConfig<float> config;
        config.Start = intensity;
        config.End = 0.0f;
        config.Duration = duration;
        config.Ease = Tweening::EasingType::EaseOutQuad;

        config.Setter = [transform, currentOffset](float val)
        {
            transform->LocalTransform.AddPosition(-(*currentOffset));

            if (val <= 0.0f)
            {
                *currentOffset = glm::vec3(0.0f);
                return;
            }

            float offsetX = ((std::rand() % 100) / 100.0f - 0.5f) * 2.0f * val;
            float offsetY = ((std::rand() % 100) / 100.0f - 0.5f) * 2.0f * val;
            *currentOffset = glm::vec3(offsetX, offsetY, 0.0f);

            transform->LocalTransform.AddPosition(*currentOffset);
        };

        tweenComp->ActiveTweens.push_back(std::make_shared<Tween<float>>(config));
    }

    void Spin(TweenComponent* tweenComponent, Transform3D* transform, float duration)
    {
        TweenConfig<float> config;
        config.Start = 0.0f;
        config.End = glm::pi<float>() * 2.0f;
        config.Duration = duration;
        config.Ease = Tweening::EasingType::EaseInOutSine;

        config.Setter = [transform, config](float angle)
        {
            transform->LocalTransform.SetRotationYawPitchRoll(angle, 0.0f, 0.0f);
        };

        tweenComponent->ActiveTweens.push_back(std::make_shared<Tween<float>>(config));
    }
}