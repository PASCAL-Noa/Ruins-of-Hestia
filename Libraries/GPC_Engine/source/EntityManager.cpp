#include "EntityManager.h"

namespace GPC {

    EntityManager::EntityManager() :
        m_LivingEntitiesCount(0)
    {
        for (uint64_t i = 0; i < MAX_ENTITIES; ++i) {
            m_AvailableEntityID.push(i);
        }
        for (uint64_t i = 0; i < MAX_ENTITIES; ++i) {
            m_LivingEntities[i] = MAX_ENTITIES;
        }
    }

    EntityManager::~EntityManager() {

    }

    EntityID EntityManager::CreateEntity() {
        if (m_LivingEntitiesCount >= MAX_ENTITIES) {
            GPC_WARNING << "You Exceeded max entity count (undefined behaviour) !" << ENDL;
            return 0;
        }

        EntityID eid = m_AvailableEntityID.front();
        m_AvailableEntityID.pop();

        m_LivingEntities[m_LivingEntitiesCount] = eid;
        m_Indices[eid] = m_LivingEntitiesCount;
        ++m_LivingEntitiesCount;

        return eid;
    }

    void EntityManager::DestroyEntity(EntityID eid) {
        if (m_LivingEntities[m_Indices[eid]] == MAX_ENTITIES) {
            GPC_WARNING << "You are trying to destroy an entity already destroyed ! (ID=" << eid << ")" << ENDL;
            return;
        }
        if (eid >= MAX_ENTITIES) {
            GPC_WARNING << "You are trying to destroy an invalid EntityID ! (ID=" << eid << ") >= 5000" << ENDL;
            return;
        }

        m_Signatures[eid].reset();
        m_AvailableEntityID.push(eid);

        uint32_t index_destroyed = m_Indices[eid];
        uint32_t index_last = m_Indices[m_LivingEntities[m_LivingEntitiesCount - 1]];
        m_LivingEntities[index_destroyed] = m_LivingEntities[index_last];

        m_Indices[m_LivingEntities[m_LivingEntitiesCount - 1]] = index_destroyed;
        m_Indices[eid] = MAX_ENTITIES;

        --m_LivingEntitiesCount;
    }

    void EntityManager::SetSignature(EntityID eid, Signature signature) {
        if (eid >= MAX_ENTITIES) {
            GPC_WARNING << "You are trying to access an invalid EntityID ! (ID=" << eid << ") >= 5000" << ENDL;
            return;
        }

        m_Signatures[eid] = signature;
    }

    Signature EntityManager::GetSignature(EntityID eid) {
        if (eid >= MAX_ENTITIES) {
            GPC_WARNING << "You are trying to access an invalid EntityID ! (ID=" << eid << ") >= 5000" << ENDL;
            return 0;
        }

        return m_Signatures[eid];
    }

    bool EntityManager::IsEntityAlive(EntityID eid) {
        if (eid == MAX_ENTITIES) return false;
        return m_Indices[eid] != MAX_ENTITIES;
    }
} // GPC