#pragma once
#include "../BuildingBehavior.h"

namespace GPC
{
    class HospitalBehavior : public BuildingBehavior
    {
        public:

        INHERIT_SPECIFIC_BEHAVIOR_CONSTRUCTOR(HospitalBehavior, BuildingBehavior);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void OnInteract() override;
        void OnNextCycle() override ;
        void CloseOpened() override;
        private:

    };
}
