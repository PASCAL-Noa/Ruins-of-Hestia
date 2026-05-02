#pragma once
#include "Behavior.h"
#include "GLM_Include.h"
#include "TransformComponents.h"

namespace GPC {

    struct AutoRotateDebug : Behavior {
        glm::vec3 delta_yaw_pitch_roll{0.0f, 0.0f, 0.0f};
        glm::vec3 current_yaw_pitch_roll{0, 0, 0};
        INHERIT_BEHAVIOR_CONSTRUCTOR(AutoRotateDebug);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

    private:
        Transform3D* mp_Transform = nullptr;



    };

} // GPC