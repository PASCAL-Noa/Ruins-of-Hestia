#include "IsoCamera.h"

#include "CameraSystem.h"
#include "Inputs.h"
#include "Scene.h"

namespace GPC {
    void IsoCamera::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);

        pTransform = BEHAVIOR_COMPONENT(Transform3D);
        pTransform->LocalTransform.SetRotationYawPitchRoll(glm::radians(CameraAngleYaw), glm::radians(CameraAnglePitch), 0.0);
    }

    void IsoCamera::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        float dt = static_cast<float>(pCtx->pClock->GetDeltaTime());
        float CameraMovementSpeed = MovementSpeed * dt;
       glm::vec3 forward = pTransform->LocalTransform.GetForward();
       glm::vec3 right = pTransform->LocalTransform.GetRight();

        forward.y = 0;
        right.y = 0;

        forward = glm::normalize(forward) * CameraMovementSpeed;
        right   = glm::normalize(right) * CameraMovementSpeed;

        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::S)) {
            pTransform->LocalTransform.AddPosition(forward);
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::W)) {
            pTransform->LocalTransform.AddPosition(-forward);
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::D)) {
            pTransform->LocalTransform.AddPosition(right);
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::A)) {
            pTransform->LocalTransform.AddPosition(-right);
        }
    }
} // GPC