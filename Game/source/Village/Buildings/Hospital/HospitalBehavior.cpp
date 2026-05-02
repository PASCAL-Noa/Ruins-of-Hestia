#include "Village/Buildings/Hospital/HospitalBehavior.h"

namespace GPC
{
    void HospitalBehavior::OnCreate(const BehaviorCreateContext* pCtx){
        BuildingBehavior::OnCreate(pCtx);
    }
    void HospitalBehavior::OnUpdate(const BehaviorUpdateContext* pCtx){
        BuildingBehavior::OnUpdate(pCtx);

    }

    void HospitalBehavior::OnInteract()
    {
        BuildingBehavior::OnInteract();

    }

    void HospitalBehavior::OnNextCycle()
    {
    }

    void HospitalBehavior::CloseOpened()
    {
        BuildingBehavior::CloseOpened();
    }
}
