#include "Render3DSystem.h"

#include "Inputs.h"
#include "RenderObject3D.h"
#include "Scene.h"
#include "Window.h"

namespace GPC {

    void RenderableComponent::SetTexture(std::string const& arrayName, std::string const& diffuse, std::string const& normal,
        std::string const& height)
    {
        SetTexture(arrayName, diffuse, normal);
        RenderMaterial.HeightMap = GPC_TEXTURE_ID(arrayName, height);
    }

    void RenderableComponent::SetTexture(std::string const &arrayName, std::string const &diffuse,
        std::string const &normal)
    {
        SetTexture(arrayName, diffuse);
        RenderMaterial.NormalMap = GPC_TEXTURE_ID(arrayName, normal);
    }

    void RenderableComponent::SetTexture(std::string const& arrayName, std::string const& diffuse)
    {
        RenderMaterial.TextureArray = GPC_TEXTURE_ARRAY(arrayName)->Array.GetUploadId();
        RenderMaterial.Diffuse = GPC_TEXTURE_ID(arrayName, diffuse);
    }

    void Render3DSystem::OnUpdate() {
        System::OnUpdate();

        Update3DObjects();

        Update3DBatching();

    }

    void Render3DSystem::Update3DObjects()
    {

        CBlockPtr<Render3DComponent> render_component_block = m_Ctx.pScene->GetComponentBlock<Render3DComponent>();
        Render3DComponent* render_components = render_component_block->ComponentData();
        EntityID* entities = render_component_block->EntitiesData();
        uint32_t entityCount = render_component_block->GetEntityCount();

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            if (render_components[i].IsEnable == false || render_components[i].pGeometry == nullptr) continue;

            Transform3D* transform = m_Ctx.pScene->GetComponent<Transform3D>(entities[i]);

            m_RenderContainer.Transform = &transform->GetWorldMatrix();
            m_RenderContainer.Mesh = render_components[i].pGeometry;
            m_RenderContainer.Material_ = &render_components[i].RenderMaterial;
            if (render_components[i].pGraphicProgram == nullptr) render_components[i].pGraphicProgram = &GPC_GRAPHIC_PROGRAM("Default Program")->Program;
            m_RenderContainer.GraphicProgram = render_components[i].pGraphicProgram;
            m_RenderContainer.UseShadow = render_components[i].UseShadow;

            m_Ctx.pWindow->DrawObject(m_RenderContainer);

        }

    }

    void Render3DSystem::Update3DBatching()
    {

        CBlockPtr<Batching3DComponent> render_component_block = m_Ctx.pScene->GetComponentBlock<Batching3DComponent>();
        Batching3DComponent* render_components = render_component_block->ComponentData();
        EntityID* entities = render_component_block->EntitiesData();
        uint32_t entityCount = render_component_block->GetEntityCount();

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            if (render_components[i].IsEnable == false || render_components[i].pGeometry == nullptr) continue;

            m_RenderObjectBatched.BatchedObjects    = render_components[i].Batched3DObjects.data();
            m_RenderObjectBatched.EntityCount       = render_components[i].Batched3DObjects.size();
            if (render_components[i].pGraphicProgram == nullptr) render_components[i].pGraphicProgram = &GPC_GRAPHIC_PROGRAM("Default Program")->Program;
            m_RenderObjectBatched.GraphicProgram    = render_components[i].pGraphicProgram;
            m_RenderObjectBatched.UseShadow         = render_components[i].UseShadow;
            m_RenderObjectBatched.Mesh              = render_components[i].pGeometry;
            m_RenderObjectBatched.Material_          = &render_components[i].RenderMaterial;

            m_Ctx.pWindow->DrawObjects(m_RenderObjectBatched);

        }

    }
} // GPC