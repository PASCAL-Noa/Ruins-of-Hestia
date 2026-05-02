#include "Tweens.h"
#include <glm/glm.hpp>

namespace GPC::Tweening
{

    EasingFunction GetFunction(EasingType type)
    {
        switch(type)
        {
            case EasingType::Linear:            return Linear;

            case EasingType::EaseInSine:        return SineIn;
            case EasingType::EaseOutSine:       return SineOut;
            case EasingType::EaseInOutSine:     return SineInOut;

            case EasingType::EaseInCubic:       return CubicIn;
            case EasingType::EaseOutCubic:      return CubicOut;
            case EasingType::EaseInOutCubic:    return CubicInOut;

            case EasingType::EaseInQuint:       return QuintIn;
            case EasingType::EaseOutQuint:      return QuintOut;
            case EasingType::EaseInOutQuint:    return QuintInOut;

            case EasingType::EaseInCirc:        return CircIn;
            case EasingType::EaseOutCirc:       return CircOut;
            case EasingType::EaseInOutCirc:     return CircInOut;

            case EasingType::EaseInElastic:     return ElasticIn;
            case EasingType::EaseOutElastic:    return ElasticOut;
            case EasingType::EaseInOutElastic:  return ElasticInOut;

            case EasingType::EaseInQuad:        return QuadIn;
            case EasingType::EaseOutQuad:       return QuadOut;
            case EasingType::EaseInOutQuad:     return QuadInOut;

            case EasingType::EaseInQuart:       return QuartIn;
            case EasingType::EaseOutQuart:      return QuartOut;
            case EasingType::EaseInOutQuart:    return QuartInOut;

            case EasingType::EaseInExpo:        return ExpoIn;
            case EasingType::EaseOutExpo:       return ExpoOut;
            case EasingType::EaseInOutExpo:     return ExpoInOut;

            case EasingType::EaseInBack:        return BackIn;
            case EasingType::EaseOutBack:       return BackOut;
            case EasingType::EaseInOutBack:     return BackInOut;

            case EasingType::EaseInBounce:      return BounceIn;
            case EasingType::EaseOutBounce:     return BounceOut;
            case EasingType::EaseInOutBounce:   return BounceInOut;
            case EasingType::TransiFeuille:     return TransiFeuilleEasing;

            default: return Linear;
        }
    }

    float Linear(float t)       { return t; }

    float SineIn(float t)       { return 1.0f - glm::cos((t * PI) / 2.0f); }
    float SineOut(float t)      { return glm::sin((t * PI) / 2.0f); }
    float SineInOut(float t)    { return -(glm::cos(PI * t) - 1.0f) / 2.0f; }

    float CubicIn(float t)      { return t * t * t; }
    float CubicOut(float t)     { t -= 1.0f; return t * t * t + 1.0f; }
    float CubicInOut(float t)   { return t < 0.5f ? 4.0f * t * t * t : 1.0f - glm::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f; }

    float QuintIn(float t)      { return t * t * t * t * t; }
    float QuintOut(float t)     { return glm::sqrt(1.0f - glm::pow(t - 1.0f, 2.0f)); }
    float QuintInOut(float t)   { return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f - glm::pow(-2.0f * t + 2.0f, 5.0f) / 2.0f; }

    float CircIn(float t)       { return 1.0f - glm::sqrt(1.0f - glm::pow(t, 2.0f)); }
    float CircOut(float t)      { return glm::sqrt(1.0f - glm::pow(t - 1.0f, 2.0f)); }
    float CircInOut(float t)    { return t < 0.5f ? (1.0f - glm::sqrt(1.0f - glm::pow(2.0f * t, 2.0f))) / 2.0f  : (glm::sqrt(1.0f - glm::pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f; }

    float TransiFeuilleEasing(float t) { return 1.35f * t * t - 0.35f * t; }

    float ElasticIn(float t)
    {
        float c4 = (2.0f * PI) / 3.0f;
        return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : -glm::pow(2.0f, 10.0f * t - 10.0f) * glm::sin((t * 10.0f - 10.75f) * c4);
    }
    float ElasticOut(float t)
    {
        float c4 = (2.0f * PI) / 3.0f;
        return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : glm::pow(2.0f, -10.0f * t) * glm::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
    }
    float ElasticInOut(float t)
    {
        float c5 = (2.0f * PI) / 4.5f;
        return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : t < 0.5f ? -(glm::pow(2.0f, 20.0f * t - 10.0f) * glm::sin((20.0f * t - 11.125f) * c5)) / 2.0f : (glm::pow(2.0f, -20.0f * t + 10.0f) * glm::sin((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
    }

    float QuadIn(float t)       { return t * t; }
    float QuadOut(float t)      { return 1.0f - (1.0f - t) * (1.0f - t);  }
    float QuadInOut(float t)    { return t < 0.5f ? 2.0f * t * t : 1.0f - glm::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f; }

    float QuartIn(float t)      { return t * t * t * t; }
    float QuartOut(float t)     { return 1.0f - glm::pow(t - 1.0f, 4.0f); }
    float QuartInOut(float t)   { return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - glm::pow(-2.0f * t + 2.0f, 4.0f) / 2.0f; }

    float ExpoIn(float t)       { return t == 0.0f ? 0.0f : glm::pow(2.0f, 10.0f * t - 10.0f); }
    float ExpoOut(float t)      { return t == 1.0f ? 1.0f : 1.0f - glm::pow(2.0f, -10.0f * t); }
    float ExpoInOut(float t)    { return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : t < 0.5f ? glm::pow(2.0f, 20.0f * t - 10.0f) / 2.0f : (2.0f - glm::pow(2.0f, -20.0f * t + 10.0f)) / 2.0f; }

    float BackIn(float t)
    {
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        return c3 * t * t * t - c1 * t * t;
    }
    float BackOut(float t)
    {
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        return 1.0f + c3 * glm::pow(t - 1.0f, 3.0f) + c1 * glm::pow(t - 1.0f, 2.0f);
    }
    float BackInOut(float t)
    {
        float c1 = 1.70158f;
        float c2 = c1 * 1.525f;
        return t < 0.5f ? (glm::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f  : (glm::pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }

    float BounceIn(float t)     { return 1.0f - BounceOut(1.0f - t); }
    float BounceOut(float t)
    {
        float n1 = 7.5625f;
        float d1 = 2.75f;

        if (t < 1.0f / d1) return n1 * t * t;
        if (t < 2.0f / d1) return n1 * (t -= 1.5f / d1) * t + 0.75f;
        if (t < 2.5f / d1) return n1 * (t -= 2.25f / d1) * t + 0.9375f;

        return n1 * (t -= 2.625f / d1) * t + 0.984375f;
    }
    float BounceInOut(float t)  { return t < 0.5f ? (1.0f - BounceOut(1.0f - 2.0f * t)) / 2.0f : (1.0f + BounceOut(2.0f * t - 1.0f)) / 2.0f; }
}
