#include "Village/Buildings/Farm/FarmBehavior.h"
#include "Village/Buildings/Farm/Farm.h"

namespace GPC
{
    void FarmBehavior::OnCreate(const BehaviorCreateContext* pCtx)
    {
        BuildingBehavior::OnCreate(pCtx);

    }

    void FarmBehavior::OnStart(const BehaviorCreateContext* pCtx)
    {
        Behavior::OnStart(pCtx);

        CreateBaseInteractionUI(pCtx, &m_Canvas);
        m_Canvas->IsEnable = false;
    }

    void FarmBehavior::OnUpdate(const BehaviorUpdateContext* pCtx)
    {
        BuildingBehavior::OnUpdate(pCtx);
    }

    void FarmBehavior::OnInteract()
    {
        BuildingBehavior::OnInteract();
        m_Canvas->IsEnable = !m_Canvas->IsEnable;
    }

    void FarmBehavior::OnNextCycle()
    {

        VillageManager_->GetInventory()
            .AddResource(Building_->DependentTo.ResourceId, Building_->DependentTo.Amount);
    }

    void FarmBehavior::CloseOpened()
    {
        BuildingBehavior::CloseOpened();
        m_Canvas->IsEnable = false;
    }
}
