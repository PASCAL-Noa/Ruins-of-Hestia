#ifndef GPC_SYSTEM_MANAGER_H
#define GPC_SYSTEM_MANAGER_H

#include "System.h"

namespace GPC {
    class SystemManager {
        std::array<std::shared_ptr<System>, MAX_SYSTEMS> m_Systems;

        template<typename System_t>  requires(std::is_base_of_v<System, System_t>)
        void InitializeSystem() {
            m_Systems[static_cast<uint64_t>(System_t::Type) - 1].reset(new System_t());
        }

    public:

        SystemManager();
        virtual ~SystemManager() = default;

        template<typename System_t>  requires(std::is_base_of_v<System, System_t>)
        std::shared_ptr<System_t> GetSystem() {
            return std::static_pointer_cast<System_t>(m_Systems[static_cast<uint64_t>(System_t::Type) - 1]);
        }

        void UpdateAllCtxScene(Scene* scene) const {
            SystemContext ctx{};
            ctx.pScene = scene;
            for (auto system : m_Systems) {
                system->OnContextUpdate(&ctx);
            }
        }

        template<typename System_t>  requires(std::is_base_of_v<System, System_t>)
        void BeginUpdateSystem(SystemContext* pCtx) {
            m_Systems[static_cast<uint64_t>(System_t::Type) - 1]->OnContextUpdate(pCtx);
            m_Systems[static_cast<uint64_t>(System_t::Type) - 1]->OnBeginUpdate();
        }

        template<typename System_t>  requires(std::is_base_of_v<System, System_t>)
        void UpdateSystem() {
            m_Systems[static_cast<uint64_t>(System_t::Type) - 1]->OnUpdate();
        }

        template<typename System_t>  requires(std::is_base_of_v<System, System_t>)
        void LateUpdateSystem() {
            m_Systems[static_cast<uint64_t>(System_t::Type) - 1]->OnLateUpdate();
        }

        template<typename System_t>  requires(std::is_base_of_v<System, System_t>)
        void EndUpdateSystem() {
            m_Systems[static_cast<uint64_t>(System_t::Type) - 1]->OnEndUpdate();
        }

    };

} // GPC

#endif // GPC_SYSTEM_MANAGER_H
