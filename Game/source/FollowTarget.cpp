#include "FollowTarget.h"

#include "Scene.h"

namespace GPC {
    void FollowTarget::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);
        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
        mp_Camera = pCtx->pScene->GetComponent<CameraComponent>(GetEntityID());
    }

    void FollowTarget::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);
        mp_Transform->LocalTransform.SetPosition(pTarget->LocalTransform.GetPosition() + Offset);
    }

    void FollowTarget::OnUpdate(const BehaviorUpdateContext *pCtx) {
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

        auto scroll = GPC::Inputs::GetScrollOffsetY();
        if (scroll < -FLT_EPSILON || scroll > FLT_EPSILON) {
            const float ZOOM_SPEED = 1.75f * ZoomScale;
            const float ZOOM_MIN = BaseMinZoom * ZoomScale;
            const float ZOOM_MAX = BaseMaxZoom * ZoomScale;
            Zoom -= scroll * ZOOM_SPEED * pCtx->pClock->GetDeltaTime();
            if (Zoom < ZOOM_MIN) Zoom = ZOOM_MIN;
            if (Zoom > ZOOM_MAX) Zoom = ZOOM_MAX;
            mp_Camera->UseOrthographic(Zoom);
        }

    }

    void FollowTarget::SetVillageCameraInfo()
    {
        ZoomScale = 5.f;
        BaseMaxZoom = 3.0f;
    }
} // GPC
