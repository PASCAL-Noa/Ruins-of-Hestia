#pragma once
#include "../BuildingBehavior.h"
#include "Village/VillageManager.h"

namespace GPC
{
    class Farm;

    class FarmBehavior : public BuildingBehavior
    {
        public:

        INHERIT_SPECIFIC_BEHAVIOR_CONSTRUCTOR(FarmBehavior, BuildingBehavior);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnStart(const BehaviorCreateContext* pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void OnInteract() override;
        void OnNextCycle() override;
        void CloseOpened() override;

    private:
        UiCanvas* m_Canvas = nullptr;
    };
}
