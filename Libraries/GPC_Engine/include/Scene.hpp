#pragma once
#include "Scene.h"

namespace GPC {

    template<typename Component_t> requires(std::is_base_of_v<Component, Component_t>)
    Component_t * Scene::AddComponent(EntityID eid) {
        Component_t* ret;
        if(m_EntityManager.HasSignature<Component_t>(eid)) {
            ret = GetComponent<Component_t>(eid);
        }
        else {
            m_EntityManager.SetSignature(eid, m_EntityManager.GetSignature(eid).to_ullong() | 1ull << static_cast<uint64_t>(Component_t::Type));
            ret = m_ComponentBlockManager.AddComponent<Component_t>(eid);

        }
        return ret;
    }

    template<typename Component_t> requires (std::is_base_of_v<Component, Component_t>)
    bool Scene::HasComponent(EntityID eid)
    {
        return m_EntityManager.HasSignature<Component_t>(eid);
    }

    template<typename Component_t> requires (std::is_base_of_v<Component, Component_t>)
    void Scene::RemoveComponent(EntityID eid) {
        m_ComponentBlockManager.RemoveComponent<Component_t>(eid);
    }

    template<typename Component_t> requires (std::is_base_of_v<Component, Component_t>)
    Component_t * Scene::GetComponent(EntityID eid) {
        return &m_ComponentBlockManager.GetComponent<Component_t>(eid);
    }

    template<typename Component_t> requires (std::is_base_of_v<Component, Component_t>)
    CBlockPtr<Component_t> Scene::GetComponentBlock() {
        return m_ComponentBlockManager.GetComponentBlock<Component_t>();
    }

    template<typename Behavior_t> requires (std::is_base_of_v<Behavior, Behavior_t>)
    Behavior_t * Scene::AddBehavior(EntityID eid) {
        BehaviorCreateContext ctx{};
        ctx.pScene = this;
        ctx.pClock = &m_Clock;
        return m_BehaviorBlockManager.GetBlock(eid)->AddBehavior<Behavior_t>(&ctx);
    }

    template<typename Behavior_t> requires (std::is_base_of_v<Behavior, Behavior_t>)
    void Scene::RemoveBehavior(EntityID eid) {
        auto* pBehavior = GetBehavior<Behavior_t>(eid);
        if (pBehavior == nullptr) return;
        m_BehaviorBlockManager.GetBlock(eid)->RemoveBehavior(pBehavior);
    }

    template<typename Behavior_t> requires (std::is_base_of_v<Behavior, Behavior_t>)
    Behavior_t * Scene::GetBehavior(EntityID eid) {
        return m_BehaviorBlockManager.GetBlock(eid)->GetBehavior<Behavior_t>();
    }
}
