#include "FollowTargetVillage.h"

#include "Scene.h"

namespace GPC {
    void FollowTargetVillage::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);
        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
        mp_Camera = pCtx->pScene->GetComponent<CameraComponent>(GetEntityID());
    }

    void FollowTargetVillage::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);
        mp_Transform->LocalTransform.SetPosition(pTarget->LocalTransform.GetPosition() + Offset);
    }

    void FollowTargetVillage::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (pTarget == nullptr) return;

        float finalLerpSpeed = LerpSpeed * pCtx->pClock->GetDeltaTime();
        mp_Transform->LocalTransform.SetPosition(
            glm::lerp(
                mp_Transform->LocalTransform.GetPosition(),
                pTarget->LocalTransform.GetPosition() + Offset,
                { finalLerpSpeed,  finalLerpSpeed, finalLerpSpeed }
            )
        );

        if (LockZoom) {
            mp_Camera->UseOrthographic(Zoom);
            return;
        }
        auto scroll = GPC::Inputs::GetScrollOffsetY();
        if (scroll < -FLT_EPSILON || scroll > FLT_EPSILON) {
            constexpr float ZOOM_SPEED = 12.5f;
            constexpr float ZOOM_MIN = 10.0f;
            constexpr float ZOOM_MAX = 12.0f;
            Zoom -= scroll * ZOOM_SPEED * pCtx->pClock->GetDeltaTime();
            if (Zoom < ZOOM_MIN) Zoom = ZOOM_MIN;
            if (Zoom > ZOOM_MAX) Zoom = ZOOM_MAX;
            mp_Camera->UseOrthographic(Zoom);
        }

    }
} // GPC
