#include "DebugBehavior/AutoRotateDebug.h"

#include "Scene.h"

namespace GPC {

    void AutoRotateDebug::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);
        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
    }

    void AutoRotateDebug::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);
        mp_Transform->LocalTransform.SetRotationYawPitchRoll(current_yaw_pitch_roll);
        current_yaw_pitch_roll.x += pCtx->pClock->GetDeltaTime() * delta_yaw_pitch_roll.x;
        current_yaw_pitch_roll.y += pCtx->pClock->GetDeltaTime() * delta_yaw_pitch_roll.y;
        current_yaw_pitch_roll.z += pCtx->pClock->GetDeltaTime() * delta_yaw_pitch_roll.z;
    }


} // GPC