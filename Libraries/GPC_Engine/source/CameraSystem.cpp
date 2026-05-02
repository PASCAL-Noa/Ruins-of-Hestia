#include "CameraSystem.h"

#include "Scene.h"

namespace GPC {
    void CameraSystem::UpdateCurrentActive() {

        CBlockPtr<CameraComponent> camera_component_block = m_Ctx.pScene->GetComponentBlock<CameraComponent>();
        CameraComponent* camera_components = camera_component_block->ComponentData();
        uint32_t entityCount = camera_component_block->GetEntityCount();

        for (uint32_t i = 0; i < entityCount; ++i) {
            if (camera_components[i].IsEnable == false) continue;
            if (camera_components[i].Active) {
                m_CurrentCamera = i;
            }
            camera_components[i].Active = false;
        }

    }

    CameraComponent * CameraSystem::CameraActive() {
        CBlockPtr<CameraComponent> camera_component_block = m_Ctx.pScene->GetComponentBlock<CameraComponent>();
        CameraComponent* camera_components = camera_component_block->ComponentData();

        UpdateCurrentActive();

        return &camera_components[m_CurrentCamera];
    }

    void CameraSystem::OnUpdate() {
        System::OnUpdate();

        CBlockPtr<CameraComponent> camera_component_block = m_Ctx.pScene->GetComponentBlock<CameraComponent>();
        CameraComponent* camera_components = camera_component_block->ComponentData();
        EntityID* entities = camera_component_block->EntitiesData();
        uint32_t entityCount = camera_component_block->GetEntityCount();

        UpdateCurrentActive();

        Transform3D* transform = nullptr;
        if (entityCount > m_CurrentCamera && camera_components[m_CurrentCamera].IsEnable) {
            transform = m_Ctx.pScene->GetComponent<Transform3D>(entities[m_CurrentCamera]);
        } else if (entityCount > 0 && camera_components[0].IsEnable) {
            transform = m_Ctx.pScene->GetComponent<Transform3D>(entities[0]);
        }
        if (transform == nullptr) return;


        camera_components[m_CurrentCamera].Context.Transform = glm::inverse(transform->GetWorldMatrix());
        m_Ctx.pWindow->UpdateCamera(camera_components[m_CurrentCamera].Context);
    }
} // GPC