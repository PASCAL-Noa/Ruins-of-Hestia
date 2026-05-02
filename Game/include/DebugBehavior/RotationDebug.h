#pragma once
#include "Behavior.h"
#include "Inputs.h"
#include "TransformComponents.h"

namespace GPC {

    struct RotationDebug : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(RotationDebug);

        GPC::Inputs::KeyCode RotateLeftKey  = Inputs::KeyCode::ESCAPE;
        GPC::Inputs::KeyCode RotateRightKey = Inputs::KeyCode::ESCAPE;
        float RotationSpeed = 10.0f;

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

    private:
        float m_Yaw = 0.0f;
        Transform3D* mp_Transform;
    };

} // GPC