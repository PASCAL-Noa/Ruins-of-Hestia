#include "DebugBehavior/MoveDebug.h"

#include "Scene.h"
#include "Map Generation/Room.h"

namespace GPC {
    void MoveDebug::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);
        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
    }

    void MoveDebug::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        constexpr float _cos = 0.70710678f;
        constexpr float _sin = 0.70710678f;

        if (GPC::Inputs::IsKeyDown(KeyUp)) {
            mp_Transform->LocalTransform.AddPosition(-_cos * pCtx->pClock->GetDeltaTime() * MovementSpeed, 0, -_sin * pCtx->pClock->GetDeltaTime() * MovementSpeed);
        }
        if (GPC::Inputs::IsKeyDown(KeyDown)) {
            mp_Transform->LocalTransform.AddPosition(_cos * pCtx->pClock->GetDeltaTime() * MovementSpeed, 0, _sin * pCtx->pClock->GetDeltaTime() * MovementSpeed);
        }
        if (GPC::Inputs::IsKeyDown(KeyLeft)) {
            mp_Transform->LocalTransform.AddPosition(-_cos * pCtx->pClock->GetDeltaTime() * MovementSpeed, 0, _sin * pCtx->pClock->GetDeltaTime() * MovementSpeed);
        }
        if (GPC::Inputs::IsKeyDown(KeyRight)) {
            mp_Transform->LocalTransform.AddPosition(_cos * pCtx->pClock->GetDeltaTime() * MovementSpeed, 0, -_sin * pCtx->pClock->GetDeltaTime() * MovementSpeed);
        }
        if (GPC::Inputs::IsKeyDown(KeyTop)) {
            mp_Transform->LocalTransform.AddPosition(0, pCtx->pClock->GetDeltaTime() * MovementSpeed, 0);
        }
        if (GPC::Inputs::IsKeyDown(KeyBottom)) {
            mp_Transform->LocalTransform.AddPosition(0, -pCtx->pClock->GetDeltaTime() * MovementSpeed, 0);
        }

    }
} // GPC