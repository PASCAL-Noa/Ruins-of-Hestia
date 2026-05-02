#pragma once

#include "Behavior.h"
#include "Inputs.h"
#include "TransformComponents.h"

#include <cmath>

namespace GPC {

    struct MoveDebug : public Behavior {
    public:
        INHERIT_BEHAVIOR_CONSTRUCTOR(MoveDebug);

        GPC::Inputs::KeyCode KeyUp      = Inputs::KeyCode::ESCAPE;
        GPC::Inputs::KeyCode KeyDown    = Inputs::KeyCode::ESCAPE;
        GPC::Inputs::KeyCode KeyRight   = Inputs::KeyCode::ESCAPE;
        GPC::Inputs::KeyCode KeyLeft    = Inputs::KeyCode::ESCAPE;
        GPC::Inputs::KeyCode KeyTop    = Inputs::KeyCode::ESCAPE;
        GPC::Inputs::KeyCode KeyBottom    = Inputs::KeyCode::ESCAPE;
        float MovementSpeed             = 10.0f;

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;
    private:
        Transform3D* mp_Transform;
    };

} // GPC