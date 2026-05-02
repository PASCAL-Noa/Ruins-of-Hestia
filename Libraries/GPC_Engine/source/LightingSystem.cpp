#include "LightingSystem.h"

#include "Scene.h"
#include "Window.h"

namespace GPC {
    void LightingSystem::OnUpdate() {
        System::OnUpdate();

        CBlockPtr<LightingComponent> light_component_block = m_Ctx.pScene->GetComponentBlock<LightingComponent>();
        LightingComponent* light_components = light_component_block->ComponentData();
        EntityID* entities = light_component_block->EntitiesData();
        uint32_t entityCount = light_component_block->GetEntityCount();

        uint32_t id = 0;
        for (uint32_t i = 0; i < entityCount; ++i)
        {
            if (light_components[i].IsEnable == false) continue;
            Transform3D* transform = m_Ctx.pScene->GetComponent<Transform3D>(entities[i]);
            glm::mat4x4 world = transform->GetWorldMatrix();
            glm::vec3 pos{};
            pos.x = world[3].x;
            pos.y = world[3].y;
            pos.z = world[3].z;
            light_components[i].pLightData->SetPosition(pos);
            light_components[i].pLightData->SetDirection({transform->LocalTransform.GetForward().x, transform->LocalTransform.GetForward().y, transform->LocalTransform.GetForward().z, 1.0f});
            m_Ctx.pWindow->UpdateLight(light_components[i].pLightData, id);
            id++;
        }

    }
} // GPC