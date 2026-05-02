#pragma once

#include "Behavior.h"
#include "Village/Inventory.h"

namespace GPC {

    struct BackToVillageBehavior : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(BackToVillageBehavior);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        Transform3D* pPlayerTransform;
        Inventory* pInventory;
        bool UseTuto = false;
        float Start_Time = 0.0f;

        std::function<void()> OnInteract = nullptr;

    private:
        Transform3D* mp_Transform;
        bool m_EndOfMissionTutoHasProc = false;
    };

} // GPC