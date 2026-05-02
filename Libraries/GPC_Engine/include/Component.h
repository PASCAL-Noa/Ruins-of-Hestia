#ifndef GPC_COMPONENT_H
#define GPC_COMPONENT_H

#include "ECS_Defines.h"

#define BIND_COMPONENT(type) static constexpr GPC::ComponentType Type = GPC::ComponentType::type; GPC::ComponentType GetType() const override  { return GPC::ComponentType::type; };  GPC::Signature GetSignature() const override  { return GPC::Signature(COMPONENT_MASK(type)); }
#define AUTO_COMPONENT_CONSTRUCTOR(class) class(EntityID eid) : Component(eid) {}; class() : Component() {};

namespace GPC {

    struct Component {
        static constexpr bool kStableSlot = false;

        bool IsEnable = true;
        virtual ~Component() = default;
        [[nodiscard]] virtual ComponentType GetType() const = 0;
        [[nodiscard]] virtual Signature GetSignature() const = 0;

        EntityID GetEntityID() const { return m_EID; }
        Component(EntityID eid) : m_EID(eid) { }
        Component() : m_EID(MAX_ENTITIES) { }
    private:
        EntityID m_EID = MAX_ENTITIES;
    };

} // GPC

#endif // GPC_COMPONENT_H
