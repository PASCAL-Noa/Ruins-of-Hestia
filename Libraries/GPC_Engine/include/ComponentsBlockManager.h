#ifndef GPC_COMPONENTS_BLOCK_MANAGER_H
#define GPC_COMPONENTS_BLOCK_MANAGER_H

#include "ComponentBlock.h"

namespace GPC {

    template<typename Component_t>
    using CBlockPtr = std::shared_ptr<CBlock<Component_t>>;

    class ComponentsBlockManager {
        std::array<std::shared_ptr<BaseComponentBlock>, MAX_COMPONENTS> m_ComponentBlocks;

        template<typename Component_t>  requires(std::is_base_of_v<Component, Component_t>)
        void InitializeArrays() {
            m_ComponentBlocks[static_cast<uint64_t>(Component_t::Type) - 1].reset(new ComponentBlock<Component_t>());
        }

    public:
        ComponentsBlockManager();
        virtual ~ComponentsBlockManager() = default;

        template<typename Component_t> requires(std::is_base_of_v<Component, Component_t>)
        Component_t* AddComponent(EntityID eid) {
            return GetComponentBlock<Component_t>()->CreateDefaultComponent(eid);
        }

        template<typename Component_t> requires(std::is_base_of_v<Component, Component_t>)
        Component_t* AddComponent(EntityID eid, const Component_t& component) {
            return GetComponentBlock<Component_t>()->CreateComponent(eid, component);
        }

        template<typename Component_t> requires(std::is_base_of_v<Component, Component_t>)
        void RemoveComponent(EntityID eid) {
            GetComponentBlock<Component_t>()->DestroyComponent(eid);
        }

        void RemoveComponent(EntityID eid, uint32_t component_type_id) {
            m_ComponentBlocks[component_type_id]->DestroyComponent(eid);
        }

        template<typename Component_t> requires(std::is_base_of_v<Component, Component_t>)
        Component_t& GetComponent(EntityID eid) {
            return GetComponentBlock<Component_t>()->GetComponent(eid);
        }

        template<typename Component_t>  requires(std::is_base_of_v<Component, Component_t>)
        CBlockPtr<Component_t> GetComponentBlock() {
            return std::static_pointer_cast<ComponentBlock<Component_t>>(m_ComponentBlocks[static_cast<uint64_t>(Component_t::Type) - 1]);
        }
    };
} // GPC

#endif // GPC_COMPONENTS_BLOCK_MANAGER_H
