#include "FreeCamera.h"

#include "Inputs.h"
#include "Scene.h"

namespace GPC {
    void FreeCamera::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);

        pTransform = BEHAVIOR_COMPONENT(Transform3D);
    }

    void FreeCamera::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        float dt = static_cast<float>(pCtx->pClock->GetDeltaTime());
        float CameraAngularSpeed = Sensibility * 0.0001f;
        float CameraMovementSpeed = MovementSpeed * dt;

        float dt_x = GPC::Inputs::GetDeltaMouseX();
        float dt_y = GPC::Inputs::GetDeltaMouseY();

        // Rotate the Camera
        CameraAngleYaw -= dt_x * CameraAngularSpeed;
        CameraAnglePitch -= dt_y * CameraAngularSpeed;
        if (CameraAnglePitch >= MaxAnglePitch) CameraAnglePitch = MaxAnglePitch;
        if (CameraAnglePitch <= MinAnglePitch) CameraAnglePitch = MinAnglePitch;

        pTransform->LocalTransform.SetRotationYawPitchRoll(CameraAngleYaw, CameraAnglePitch, 0.0);

        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::W)) {
            pTransform->LocalTransform.AddPosition(-pTransform->LocalTransform.GetForward() * CameraMovementSpeed);
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::S)) {
            pTransform->LocalTransform.AddPosition(pTransform->LocalTransform.GetForward() * CameraMovementSpeed);
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::A)) {
            pTransform->LocalTransform.AddPosition(-pTransform->LocalTransform.GetRight() * CameraMovementSpeed);
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::D)) {
            pTransform->LocalTransform.AddPosition(pTransform->LocalTransform.GetRight() * CameraMovementSpeed);
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::SPACE)) {
            pTransform->LocalTransform.AddPosition(pTransform->LocalTransform.GetUp() * CameraMovementSpeed);
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::LEFT_CONTROL)) {
            pTransform->LocalTransform.AddPosition(-pTransform->LocalTransform.GetUp() * CameraMovementSpeed);
        }
    }
} // GPC