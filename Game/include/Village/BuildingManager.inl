#include "Render3DSystem.h"
#include "Buildings/LifeTree/LifeTreeBehavior.h"
#include "Village/Map/Map.h"  

namespace GPC
{
    template<typename B_Class, typename Behavior>
    EntityID BuildingManager::SpawnBuilding(glm::ivec2 position, BuildingBehavior** out) const
    {
        EntityID entity = m_Scene->CreateEntityAs3D();

        Building* buildingLogic = new B_Class();
        buildingLogic->SetEntityID(entity);

        EntityID sprite = m_Scene->CreateEntityAs3D();
        Transform3D* transformSprite = m_Scene->GetComponent<Transform3D>(sprite);
        glm::ivec2 size = buildingLogic->GetSize();

        const float texW    = 1220.0f;
        const float texH    = 1683.0f;
        const float perspectiveRatio = 1.4f;
        const float scale   = buildingLogic->GetScale();

        float spriteW = (float)size.x;
        float spriteH = spriteW * (texH / texW) * perspectiveRatio;
        transformSprite->LocalTransform.SetScale({ spriteW * scale, 1.0f, spriteH * scale });

        // Position + rotation d'abord. Y projetee sur le sol tilte pour visuel coherent.
        const float spriteX = position.x + buildingLogic->Offset.x;
        const float spriteZ = position.y + buildingLogic->Offset.y;
        const float spriteY = GetMapGroundYAt(spriteX, spriteZ) + 1.0f;
        transformSprite->LocalTransform.SetPosition(glm::vec3(spriteX, spriteY, spriteZ));
        transformSprite->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.5f, GPC_PI);

        const float depthFactor   = buildingLogic->SpriteDepthFactor;
        const float lateralFactor = buildingLogic->SpriteLateralFactor;
        transformSprite->LocalTransform.AddPosition(-transformSprite->LocalTransform.GetForward() * size.y * scale * depthFactor);
        transformSprite->LocalTransform.AddPosition(transformSprite->LocalTransform.GetRight() * size.x * scale * lateralFactor);

        Render3DComponent* renderer = m_Scene->AddComponent<Render3DComponent>(sprite);
        renderer->pGeometry = GPC_MESH("PlaneDS");
        renderer->UseShadow = true;
        renderer->SetTexture("1220x1683", buildingLogic->GetTexturePath());

        Collision3DComponent* collider = m_Scene->AddComponent<Collision3DComponent>(entity);
        Transform3D* transform = m_Scene->GetComponent<Transform3D>(entity);
        const float entityX = position.x + buildingLogic->GetSize().x / 2.0f;
        const float entityZ = position.y + buildingLogic->GetSize().y / 2.0f;
        const float entityY = GetMapGroundYAt(entityX, entityZ) + 1.0f;
        const float spriteHeight = static_cast<float>(buildingLogic->GetSize().x) * (1683.0f / 1220.0f) * 1.4f * scale;
        transform->LocalTransform.SetPosition(glm::vec3(entityX, entityY, entityZ));
        transform->LocalTransform.SetScale(glm::vec3(buildingLogic->GetSize().x * 0.8f, spriteHeight * 0.3f, buildingLogic->GetSize().y * 0.8f));
        collider->Offset = glm::vec3(0.0f, spriteHeight * 0.15f - 0.5f, 0.0f);

        BuildingBehavior* behavior      = buildingLogic->Create<Behavior>(m_Scene);
        if (out != nullptr) (*out)      = behavior;
        behavior->Building_             = buildingLogic;
        behavior->VillageManager_       = m_Manager;
        behavior->Position              = position;
        behavior->Render3DComponent_    = renderer;
        behavior->Collision3DComponent_ = collider;
        behavior->CurrentBasePath       = buildingLogic->GetTexturePath();

        return entity;
    }
}
