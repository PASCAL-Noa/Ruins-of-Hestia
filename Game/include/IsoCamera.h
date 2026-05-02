#pragma once

#include "Behavior.h"
#include "TransformComponents.h"

namespace GPC {

    struct IsoCamera : public Behavior {
        float CameraAngleYaw = 45;
        float CameraAnglePitch = -45;
        float MovementSpeed = 10;
        float Sensibility = 50;
        Transform3D* pTransform = nullptr;

        INHERIT_BEHAVIOR_CONSTRUCTOR(IsoCamera);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;
    };
} // GPC