#include "Collision2DSystem.h"
#include "Scene.h"


namespace  GPC
{
    void Collision2DSystem::OnUpdate()
    {
        System::OnUpdate();

        const CBlockPtr<Collision2DComponent> collision_component_block = m_Ctx.pScene->GetComponentBlock<Collision2DComponent>();
        Collision2DComponent* collision_components = collision_component_block->ComponentData();
        const EntityID* entities = collision_component_block->EntitiesData();
        const int64_t entityCount = static_cast<int64_t>(collision_component_block->GetEntityCount());

        for (int i = 0; i < entityCount; ++i) {
            collision_components[i].IsColliding = false;
        }

        for (int i = 0; i < entityCount - 1; ++i)
        {
            const Collision2DComponent* comp1 = &collision_components[i];
            const EntityID id1 = entities[i];
            for (int j = i + 1; j < entityCount; ++j)
            {
                const Collision2DComponent* comp2 = &collision_components[j];
                const EntityID id2 = entities[j];

                if (IsColliding(comp1, id1, comp2, id2) == false)
                    continue;

                //TODO : ACTIVATE BEHAVIOR
            }
        }
    }

    bool Collision2DSystem::CollisionRectRect(const Collision2DComponent* comp1, const EntityID id1, const Collision2DComponent* comp2, const EntityID id2) const
    {
        // TODO : Use Transform2D
        const Transform3D* transform1 = m_Ctx.pScene->GetComponent<Transform3D>(id1);
        const Transform3D* transform2 = m_Ctx.pScene->GetComponent<Transform3D>(id2);

        const glm::vec2 boundingBoxPos1 = glm::vec2(transform1->LocalTransform.GetPosition()) + comp1->Offset;
        const glm::vec2 boundingBoxPos2 = glm::vec2(transform2->LocalTransform.GetPosition()) + comp2->Offset;

        return (
            boundingBoxPos1.x - comp1->HalfExtends.x <= boundingBoxPos2.x + comp2->HalfExtends.x &&
            boundingBoxPos1.x + comp1->HalfExtends.x >= boundingBoxPos2.x - comp2->HalfExtends.x &&

            boundingBoxPos1.y - comp1->HalfExtends.y <= boundingBoxPos2.y + comp2->HalfExtends.y &&
            boundingBoxPos1.y + comp1->HalfExtends.y >= boundingBoxPos2.y - comp2->HalfExtends.y
        );
    }

    bool Collision2DSystem::CollisionRectPoint(const Collision2DComponent* comp1, const EntityID id1, const Collision2DComponent* comp2, const EntityID id2) const
    {
        // TODO : Use Transform2D
        const Transform3D* transform1 = m_Ctx.pScene->GetComponent<Transform3D>(id1);
        const Transform3D* transform2 = m_Ctx.pScene->GetComponent<Transform3D>(id2);

        const glm::vec2 boundingBoxPos1 = glm::vec2(transform1->LocalTransform.GetPosition()) + comp1->Offset;
        const glm::vec2 boundingBoxPos2 = glm::vec2(transform2->LocalTransform.GetPosition()) + comp2->Offset;

        return (
            boundingBoxPos2.x >= boundingBoxPos1.x - comp1->HalfExtends.x &&
            boundingBoxPos2.x <= boundingBoxPos1.x + comp1->HalfExtends.x &&
            boundingBoxPos2.y >= boundingBoxPos1.y - comp1->HalfExtends.y &&
            boundingBoxPos2.y <= boundingBoxPos1.y + comp1->HalfExtends.y
        );
    }

    bool Collision2DSystem::IsColliding(const Collision2DComponent* comp1, const EntityID id1, const Collision2DComponent* comp2, const EntityID id2) const
    {
        if (comp1 == nullptr || comp2 == nullptr) return false;

        if (comp1->IsRect)
        {
            if (comp2->IsRect)
                return CollisionRectRect(comp1, id1, comp2, id2);

            return CollisionRectPoint(comp1, id1, comp2, id2);
        }
        if (comp2->IsRect)
            return CollisionRectPoint(comp2, id2, comp1, id1);

        return false;
    }
}
