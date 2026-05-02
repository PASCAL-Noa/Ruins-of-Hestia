#ifndef GPC_SYSTEM_H
#define GPC_SYSTEM_H

#include "ECS_Defines.h"

#define BIND_SYSTEM(type)  \
    static constexpr GPC::SystemType Type = GPC::SystemType::type; \
    GPC::SystemType GetType() const override { return GPC::SystemType::type; };

#include "EntityManager.h"
#include "ComponentsBlockManager.h"
#include "RenderWindow.h"

namespace GPC {
    class Scene;

    struct SystemContext {
        RenderWindow* pWindow     = nullptr;
        Scene* pScene       = nullptr;
        Clock* pClock       = nullptr;
    };

    GPC_INHERIT_OBJECT(System) {
    protected:
        SystemContext m_Ctx;

    public:
        System() = default;
        ~System() override = default;
        [[nodiscard]] virtual SystemType GetType() const = 0;

        virtual void OnContextUpdate(SystemContext* pCtx) { m_Ctx = *pCtx; };
        virtual void OnBeginUpdate() { };
        virtual void OnUpdate() { };
        virtual void OnLateUpdate() { };
        virtual void OnEndUpdate() { };

    };

}
    // GPC

#endif // GPC_SYSTEM_H
