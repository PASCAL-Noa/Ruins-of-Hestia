#pragma once

#include "Behavior.h"
#include "TransformComponents.h"

namespace GPC {

    struct FreeCamera : public Behavior {

        float CameraAngleYaw = 0;
        float CameraAnglePitch = 0;
        float MovementSpeed = 10;
        float Sensibility = 50;

        float MinAnglePitch = -glm::pi<float>() * 0.33333334f;
        float MaxAnglePitch =  glm::pi<float>() * 0.33333334f;

        Transform3D* pTransform = nullptr;

        INHERIT_BEHAVIOR_CONSTRUCTOR(FreeCamera);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

    };

} // GPC