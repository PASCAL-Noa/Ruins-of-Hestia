#pragma once

#include "ECS_Defines.h"
#include "Component.h"

namespace GPC {
    struct Component;

    class BaseComponentBlock {
    public:
        virtual void DestroyComponent(EntityID eid) = 0;
        virtual EntityID* EntitiesData() = 0;
        virtual void* RawComponentData() = 0;
        virtual uint64_t GetEntityCount() const = 0;
        virtual ~BaseComponentBlock() = default;

        bool IsSlotAlive(uint64_t i) {
            return EntitiesData()[i] != MAX_ENTITIES;
        }
    };

    template<typename Component_t> requires(std::is_base_of_v<Component, Component_t>)
    GPC_INHERIT_OBJECT(ComponentBlock), public BaseComponentBlock {
        using ComponentID = uint64_t;

        ///
        ///  Components => | c1 | c4 | c2 | c8 | c5 | c0 | c3 | xx | xx |
        ///  EntityID   => | e1 | e4 | e2 | e8 | e5 | e0 | e3 | xx | xx |
        ///  Indices    => |  5 |  0 |  2 |  6 |  1 |  4 | xx | xx |  3 | => map[eid] => index dans le tableau continue
        ///

        std::array<Component_t, MAX_ENTITIES + 1>   m_ComponentArray;
        std::array<EntityID, MAX_ENTITIES>          m_ComponentEntityID;
        std::array<ComponentID, MAX_ENTITIES>       m_ComponentIndices;
        uint64_t m_ComponentCount;

    public:
        ComponentBlock() :
            m_ComponentArray(),
            m_ComponentEntityID(),
            m_ComponentIndices(),
            m_ComponentCount(0)
        {
            for (auto& index : m_ComponentIndices) {
                index = MAX_ENTITIES;
            }
            for (auto& components : m_ComponentArray) {
                components = { };
            }
            for (auto& entity : m_ComponentEntityID) {
                entity = MAX_ENTITIES;
            }
        }

        ~ComponentBlock() override = default;

        Component_t* CreateComponent(EntityID eid, const Component_t& component)
        {

            if (m_ComponentIndices[eid] != MAX_ENTITIES) {
                GPC_WARNING << "You are trying to add a component on an entity that already has it" << ENDL;
                return &GetComponent(eid);
            }

            m_ComponentArray[m_ComponentCount] = component;
            // Fix interlinking for Transform3D world matrix update
            if constexpr (std::is_same_v<Component_t, Transform3D>) {
                m_ComponentArray[m_ComponentCount].LocalTransform.InitForComponent(&m_ComponentArray[m_ComponentCount]);
            }
            m_ComponentEntityID[m_ComponentCount] = eid;
            m_ComponentIndices[eid] = m_ComponentCount;
            m_ComponentCount++;
            return &m_ComponentArray[m_ComponentCount - 1];
        }

        Component_t* CreateDefaultComponent(EntityID eid)
        {
            return CreateComponent(eid, Component_t(eid));
        }

        Component_t& GetComponent(EntityID eid)
        {
            if (eid >= MAX_ENTITIES || m_ComponentIndices[eid] == MAX_ENTITIES) {
                GPC_WARNING << "You are trying to access an invalid EntityID !" << ENDL;
                return m_ComponentArray[MAX_ENTITIES];
            }
            return m_ComponentArray[m_ComponentIndices[eid]];
        }

        void DestroyComponent(EntityID eid) override {
            if (eid >= MAX_ENTITIES) return;
            if (m_ComponentIndices[eid] == MAX_ENTITIES) return;

            if constexpr (Component_t::kStableSlot) {
                ComponentID slot = m_ComponentIndices[eid];
                m_ComponentArray[slot] = Component_t();
                m_ComponentEntityID[slot] = MAX_ENTITIES;
                m_ComponentIndices[eid] = MAX_ENTITIES;
                return;
            }

            if (m_ComponentCount >= 1) {
                ComponentID deletedID = m_ComponentIndices[eid];
                EntityID eid_last = m_ComponentEntityID[m_ComponentCount - 1];
                std::swap(
                    m_ComponentArray[m_ComponentCount - 1],
                    m_ComponentArray[deletedID]
                );
                std::swap(
                    m_ComponentEntityID[m_ComponentCount - 1],
                    m_ComponentEntityID[deletedID]
                );
                m_ComponentIndices[eid_last] = deletedID;
            }

            m_ComponentIndices[eid] = MAX_ENTITIES;
            --m_ComponentCount;
        }

        Component_t* ComponentData() {
            return m_ComponentArray.data();
        }

        void* RawComponentData() override {
            return m_ComponentArray.data();
        }

        EntityID* EntitiesData() override {
            return m_ComponentEntityID.data();
        }

        uint64_t GetEntityCount() const override {
            return m_ComponentCount;
        }
    };

    template<typename Component_t>
    using CBlock = ComponentBlock<Component_t>;
} // GPC

#include "ComponentBlock.hpp"