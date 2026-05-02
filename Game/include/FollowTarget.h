#pragma once

#include "Behavior.h"
#include "CameraSystem.h"

namespace GPC {

    struct FollowTarget : Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(FollowTarget);

        Transform3D* pTarget    = nullptr;
        glm::vec3 Offset        = {0, 0, 0};
        float LerpSpeed         = 1;
        float ZoomScale         = GENERAL_SCALE_EXPEDITION;
        float Zoom              = 1.5f * ZoomScale;

        float BaseMinZoom       = 1.2f;
        float BaseMaxZoom       = 2.0f;
        CameraComponent* mp_Camera = nullptr;

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void SetVillageCameraInfo();
    private:

        Transform3D* mp_Transform;
    };

} // GPC