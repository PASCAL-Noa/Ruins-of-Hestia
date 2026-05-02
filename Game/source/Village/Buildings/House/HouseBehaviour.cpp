#include "Village/Buildings/House/HouseBehaviour.h"

#include "Village/VillageManager.h"

namespace GPC
{
    void HouseBehaviour::OnCreate(const BehaviorCreateContext* pCtx){
        BuildingBehavior::OnCreate(pCtx);

    }

    void HouseBehaviour::OnStart(const BehaviorCreateContext *pCtx)
    {
        BuildingBehavior::OnStart(pCtx);

        CreateBaseInteractionUI(pCtx, &m_Canvas);
        m_Canvas->IsEnable = false;
    }

    void HouseBehaviour::OnUpdate(const BehaviorUpdateContext* pCtx){
        BuildingBehavior::OnUpdate(pCtx);
    }

    void HouseBehaviour::OnInteract()
    {
        BuildingBehavior::OnInteract();
        m_Canvas->IsEnable = !m_Canvas->IsEnable;
    }

    void HouseBehaviour::OnNextCycle()
    {
        VillageManager_->GetInventory()
            .AddResource(Building_->DependentTo.ResourceId, Building_->DependentTo.Amount);
    }

    void HouseBehaviour::CloseOpened()
    {
        BuildingBehavior::CloseOpened();
        m_Canvas->IsEnable = false;
    }

}
