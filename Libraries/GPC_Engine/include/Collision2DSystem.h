#ifndef GPC_COLLISION2D_SYSTEM_H
#define GPC_COLLISION2D_SYSTEM_H

#include "ECS_Defines.h"
#include "System.h"
#include "Component.h"

namespace GPC
{
    struct Collision2DComponent : public Component {
        BIND_COMPONENT(COLLISION2D);
        AUTO_COMPONENT_CONSTRUCTOR(Collision2DComponent);

        glm::vec2 HalfExtends{1, 1};
        glm::vec2 Offset{0, 0};
        bool IsRect = true;
        bool IsColliding = false;
    };

    class Collision2DSystem : public System {
    public:
        BIND_SYSTEM(COLLISION2D);

        void OnUpdate() override;

        bool CollisionRectRect(const Collision2DComponent* comp1, EntityID id1, const Collision2DComponent* comp2,  EntityID id2) const;
        bool CollisionRectPoint(const Collision2DComponent* comp1, EntityID id1, const Collision2DComponent* comp2,  EntityID id2) const;

        bool IsColliding(const Collision2DComponent* comp1, EntityID id1, const Collision2DComponent* comp2,  EntityID id2) const;
    };

}

#endif // GPC_COLLISION2D_SYSTEM_H
