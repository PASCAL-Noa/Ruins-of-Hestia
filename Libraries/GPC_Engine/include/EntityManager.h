#ifndef GPC_ENTITY_MANAGER_H
#define GPC_ENTITY_MANAGER_H

#include "ECS_Defines.h"

namespace GPC {

    class EntityManager {

        std::queue<EntityID> m_AvailableEntityID;
        std::array<Signature, MAX_ENTITIES> m_Signatures;

        std::array<EntityID, MAX_ENTITIES> m_LivingEntities;        // e1 | e3 | e4 | e2 | e0
        std::array<uint32_t, MAX_ENTITIES> m_Indices;               //  4 |  0 |  3 |  1 |  2
        uint64_t m_LivingEntitiesCount;

    public:

        EntityManager();
        ~EntityManager();

        EntityID CreateEntity();
        void DestroyEntity(EntityID eid);

        void SetSignature(EntityID eid, Signature signature);
        Signature GetSignature(EntityID eid);

        template<typename Component_t>
        bool HasSignature(EntityID eid) {
            return (GetSignature(eid).to_ullong() & (1ull << static_cast<int>(Component_t::Type)));
        }

        bool IsEntityAlive(EntityID eid);

        [[nodiscard]] uint64_t GetLivingEntitiesCount() const { return m_LivingEntitiesCount; }
        EntityID* GetLivingEntities() { return m_LivingEntities.data(); }
    };

} // GPC

#endif // GPC_ENTITY_MANAGER_H
