#pragma once
#include "../BuildingBehavior.h"

namespace GPC
{
    class HouseBehaviour : public BuildingBehavior
    {

    public:
        INHERIT_SPECIFIC_BEHAVIOR_CONSTRUCTOR(HouseBehaviour, BuildingBehavior);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void OnInteract() override;
        void OnNextCycle() override;
        void CloseOpened() override;

    private:
        UiCanvas* m_Canvas;
    };
}
