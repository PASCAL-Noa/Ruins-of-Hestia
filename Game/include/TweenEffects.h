#pragma once
#include "TweenSystem.h"
#include "TransformComponents.h"

namespace GPC::TweenEffects
{
    void Shake(TweenComponent* tweenComponent, Transform3D* transform, float duration = 0.35f, float intensity = 1.0f);
    void Spin(TweenComponent* tweenComponent, Transform3D* transform, float duration = 1.0f);
}