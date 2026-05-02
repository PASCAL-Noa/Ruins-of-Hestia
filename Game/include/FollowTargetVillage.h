#pragma once

#include "Behavior.h"
#include "CameraSystem.h"

namespace GPC {

    struct FollowTargetVillage : Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(FollowTargetVillage);

        Transform3D* pTarget    = nullptr;
        glm::vec3 Offset        = {0, 0, 0};
        float   LerpSpeed         = 1;
        float   Zoom              = 10.0f;
        bool    LockZoom          = false;
        CameraComponent* mp_Camera = nullptr;

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

    private:

        Transform3D* mp_Transform;
    };

} // GPC