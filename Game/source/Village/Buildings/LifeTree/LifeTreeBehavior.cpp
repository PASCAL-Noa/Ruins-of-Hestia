#include "Village/Buildings/LifeTree/LifeTreeBehavior.h"

namespace GPC
{
    void LifeTreeBehavior::OnCreate(const BehaviorCreateContext* pCtx){
        BuildingBehavior::OnCreate(pCtx);
    }

    void LifeTreeBehavior::OnStart(const BehaviorCreateContext *pCtx)
    {
        BuildingBehavior::OnStart(pCtx);

        CreateBaseInteractionUI(pCtx, &m_Canvas);
        m_Canvas->IsEnable = false;
    }

    void LifeTreeBehavior::OnUpdate(const BehaviorUpdateContext *pCtx)
    {
        BuildingBehavior::OnUpdate(pCtx);
    }

    void LifeTreeBehavior::OnInteract()
    {
        BuildingBehavior::OnInteract();

        if (OnRecapInteract)
        {
            OnRecapInteract();
            return;
        }

        m_Canvas->IsEnable = !m_Canvas->IsEnable;
    }

    void LifeTreeBehavior::OnNextCycle()
    {
    }

    void LifeTreeBehavior::CloseOpened()
    {
        BuildingBehavior::CloseOpened();
        m_Canvas->IsEnable = false;
    }
}
