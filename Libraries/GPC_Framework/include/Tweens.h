#pragma once

// https://easings.net/

namespace GPC
{
    namespace Tweening
    {
        constexpr float PI = 3.14159265358979323846f;

        enum class EasingType
        {
            Linear,
            EaseInSine,     EaseOutSine,    EaseInOutSine,
            EaseInCubic,    EaseOutCubic,   EaseInOutCubic,
            EaseInQuint,    EaseOutQuint,   EaseInOutQuint,
            EaseInCirc,     EaseOutCirc,    EaseInOutCirc,
            EaseInElastic,  EaseOutElastic, EaseInOutElastic,
            EaseInQuad,     EaseOutQuad,    EaseInOutQuad,
            EaseInQuart,    EaseOutQuart,   EaseInOutQuart,
            EaseInExpo,     EaseOutExpo,    EaseInOutExpo,
            EaseInBack,     EaseOutBack,    EaseInOutBack,
            EaseInBounce,   EaseOutBounce,  EaseInOutBounce,
            TransiFeuille
        };

        using EasingFunction = float(*)(float);

        EasingFunction GetFunction(EasingType type);

        float Linear(float t);

        float SineIn(float t);    float SineOut(float t);    float SineInOut(float t);
        float CubicIn(float t);   float CubicOut(float t);   float CubicInOut(float t);
        float QuintIn(float t);   float QuintOut(float t);   float QuintInOut(float t);
        float CircIn(float t);    float CircOut(float t);    float CircInOut(float t);
        float ElasticIn(float t); float ElasticOut(float t); float ElasticInOut(float t);
        float QuadIn(float t);    float QuadOut(float t);    float QuadInOut(float t);
        float QuartIn(float t);   float QuartOut(float t);   float QuartInOut(float t);
        float ExpoIn(float t);    float ExpoOut(float t);    float ExpoInOut(float t);
        float BackIn(float t);    float BackOut(float t);    float BackInOut(float t);
        float BounceIn(float t);  float BounceOut(float t);  float BounceInOut(float t);
        float TransiFeuilleEasing(float t);
    }
}