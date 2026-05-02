#include "DebugBehavior/RotationDebug.h"

#include "Scene.h"

namespace GPC {
    void RotationDebug::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);
        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
    }

    void RotationDebug::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (GPC::Inputs::IsKeyDown(RotateLeftKey)) {
            m_Yaw += pCtx->pClock->GetDeltaTime() * RotationSpeed;
        }
        if (GPC::Inputs::IsKeyDown(RotateRightKey)) {
            m_Yaw -= pCtx->pClock->GetDeltaTime() * RotationSpeed;
        }

        mp_Transform->LocalTransform.SetRotationYawPitchRoll(m_Yaw, 0, 0);
    }
} // GPC