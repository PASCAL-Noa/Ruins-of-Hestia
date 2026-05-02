#include "../../include/Samples/SceneCollision.h"
#include "Collision3DSystem.h"
#include "FreeCamera.h"
#include "LightingSystem.h"
#include "Random.h"
#include "Render3DSystem.h"
#include "Meta/MetaTypes.h"
#include "Tools/MetaGenerator.h"
#include <filesystem>

namespace GPC
{
    ErrorType SceneCollision::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();

        if (!std::filesystem::exists("Resources/Packs/common.meta"))
            Tools::GenerateCommonPack();

        if (!std::filesystem::exists("Resources/Packs/scene_collision.meta"))
            Tools::GenerateSceneCollisionPack();

        MetaError metaErr = GPC_ASSETS->LoadMeta("Resources/Packs/scene_collision.meta");
        if (metaErr != MetaError::SUCCESS)
            return ErrorType::FILE_READ_FAILED;

        MetaRuntimeContext ctx { .Window = mp_Window };
        metaErr = GPC_ASSETS->ResolveRuntimeAssets(ctx);
        if (metaErr != MetaError::SUCCESS)
            return ErrorType::FILE_READ_FAILED;

        InitProgram();

        GPC_GRAPHIC_PROGRAM("Default Program")->Program.InitializeWith(
            { &GPC_TEXTURE_ARRAY("Collision Texture")->Array },
            GPC_TEXTURE_ARRAY("Collision Texture")->TextureSampler
        );

        mp_WireFrameProgram = &GPC_GRAPHIC_PROGRAM("Default Program")->Program;
        m_BlueTextureID = GPC_TEXTURE_ID("Collision Texture", "Blue Debug");
        m_RedTextureID = GPC_TEXTURE_ID("Collision Texture", "Red Debug");

        return ErrorType::SUCCESS;
    }

    ErrorType SceneCollision::OnCreate(SceneInformation &info)
    {
        SceneDefault::OnCreate(info);
        CreateDebugOverlay();

        AddBehavior<FreeCamera>(mp_MainCamera->GetEntityID());

        glm::u32vec2 gridSize = m_SystemManager.GetSystem<Collision3DSystem>()->GridSize;
        for (int i = 0; i < 800; ++i)
        {
             glm::vec3 position = glm::vec3(
             Random::Float( 0.f,2.f * gridSize.x) - gridSize.x,
                 0,
                 Random::Float( 0.f,2.f * gridSize.y) - gridSize.y
             );

            glm::vec3 scale = glm::vec3(1.f,1.f,1.f);

            CreateCube(position, scale);
        }

        m_RayContactVisualEntity = CreateEntityAs3D();
        auto pRayContactVisualEntityRenderer = AddComponent<Render3DComponent>(m_RayContactVisualEntity);
        pRayContactVisualEntityRenderer->pGeometry = GPC_MESH("Sphere");
        pRayContactVisualEntityRenderer->RenderMaterial.Diffuse = m_BlueTextureID;
        GetComponent<Transform3D>(m_RayContactVisualEntity)->LocalTransform.SetScale(0.1f,0.1f,0.1f);

        // Entity 2 (Sphere)
        m_Entity2 = CreateEntityAs3D();
        auto pEnt2Renderer = AddComponent<Render3DComponent>(m_Entity2);
        auto pEnt2Transform = GetComponent<Transform3D>(m_Entity2);
        pEnt2Transform->LocalTransform.SetPosition(-2,0,-4);

        auto pEnt2Collider  = AddComponent<Collision3DComponent>(m_Entity2);
        pEnt2Collider->IsSphere = true;
        pEnt2Renderer->pGeometry = GPC_MESH("Sphere");
        pEnt2Collider->IsTrigger = false;
        pEnt2Collider->CollType = ColliderType::Player;


        pEnt2Renderer->RenderMaterial.Diffuse = m_BlueTextureID;



        //ShowPartiotioningBoxes();

        return ErrorType::SUCCESS;
    }

    void SceneCollision::OnDestroy()
    {

    }

    void SceneCollision::InitProgram()
    {
        // Wireframe program Loading
        {
            GPC_ASSETS->AddGraphicProgram(
                "Default Program",
                GPC_SHADER("Default Vertex"),
                GPC_SHADER("Default Fragment"),
                CullMode::FRONT,
                PolygonMode::MODE_LINE,
                {{
                    .BindingDescription = GPC::Vertex::GetBindingDescription(),
                    .AttributeDescriptions = GPC::Vertex::GetAttributeDescriptions()
                }},
                {
                    GPC::DescriptorSetInformation::ForCamera(0),
                    GPC::DescriptorSetInformation::ForTransform(1),
                    GPC::DescriptorSetInformation::ForMaterials(2),
                    GPC::DescriptorSetInformation::ForLights(3),
                    GPC::DescriptorSetInformation {
                         .Name = "images",
                         .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                         .BindingSlot = 4,
                         .DescriptorCount = 1,
                         .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                         .Sampler_ = nullptr
                    }
                },
                GRAPHICS_3D,
                mp_Window
            );

            mp_WireFrameProgram = &GPC_GRAPHIC_PROGRAM("Default Program")->Program;
        }
    }

    EntityID SceneCollision::CreateCube(glm::vec3 position, glm::vec3 scale)
    {
        EntityID ent  = CreateEntityAs3D();
        auto pEnt1Renderer      = AddComponent<Render3DComponent>(ent);
        auto pEnt1Collider  = AddComponent<Collision3DComponent>(ent);
        pEnt1Collider->IsSphere = false;
        pEnt1Collider->CollType = ColliderType::AllyProjectile;

        pEnt1Renderer->pGeometry = GPC_MESH("Cube");
        pEnt1Renderer->RenderMaterial.Diffuse = m_BlueTextureID;

        auto pEnt1Transform = GetComponent<Transform3D>(ent);
        pEnt1Transform->LocalTransform.SetPosition(position);
        pEnt1Transform->LocalTransform.SetScale(scale);

        return ent;
    }

    EntityID SceneCollision::CreateBoxPartitioning(glm::vec3 position, glm::vec3 scale)
    {
        EntityID ent  = CreateEntityAs3D();
        auto pEnt1Renderer      = AddComponent<Render3DComponent>(ent);
        pEnt1Renderer->pGeometry = GPC_MESH("Cube");
        pEnt1Renderer->RenderMaterial.Diffuse = m_RedTextureID;

        auto pEnt1Transform = GetComponent<Transform3D>(ent);
        pEnt1Transform->LocalTransform.SetPosition(position);
        pEnt1Transform->LocalTransform.SetScale(scale);

        return ent;
    }

    void SceneCollision::InitLights()
    {
        auto light0 = CreateEntityAs3D();
        auto lLight0 = AddComponent<LightingComponent>(light0);
        auto lTransform0 = GetComponent<Transform3D>(light0);
        lLight0->pLightData = GPC_POINT_LIGHT("PointWhite");
        lTransform0->LocalTransform.AddPosition(2, 3, 2);
    }

    void SceneCollision::ShowPartiotioningBoxes()
    {

        glm::vec3 cubeSize = {
            m_SystemManager.GetSystem<Collision3DSystem>()->CellSize.x,
            50,
            m_SystemManager.GetSystem<Collision3DSystem>()->CellSize.y,
        };

        for (int i = 0; i < Collision3DSystem::GridSize.x; i++)
        {
            for (int j = 0; j < Collision3DSystem::GridSize.y; j++)
            {
                glm::vec3 cubePosition =
                {
                    (i - static_cast<float>(Collision3DSystem::GridSize.x - 1) * 0.5f) * cubeSize.x ,
                    0,
                    (j - static_cast<float>(Collision3DSystem::GridSize.y - 1) * 0.5f) * cubeSize.z
                };

                CreateBoxPartitioning(cubePosition, cubeSize);
            }
        }
    }


    void SceneCollision::LoadTextures()
    {
        // Texture Loading
        GPC_ASSETS->AddTexture("Blue Debug", "Resources/Materials/blue.jpg");
        GPC_ASSETS->AddTexture("Red Debug", "Resources/Materials/red.jpg");

        GPC_ASSETS->AddTextureArray(
            "Collision Texture",
            1024, 1024,
            {
                GPC_TEXTURE("Blue Debug"),
                GPC_TEXTURE("Red Debug"),
            }
        );

        GPC_GRAPHIC_PROGRAM("Default Program")->Program.InitializeWith({&GPC_TEXTURE_ARRAY("Collision Texture")->Array}, GPC_TEXTURE_ARRAY("Collision Texture")->TextureSampler);
        m_BlueTextureID = GPC_TEXTURE_ID("Collision Texture", "Blue Debug");
        m_RedTextureID = GPC_TEXTURE_ID("Collision Texture", "Red Debug");
    }

    void SceneCollision::OnSceneUpdate3D()
    {
        SceneDefault::OnSceneUpdate3D();

        auto pEnt2Transform = GetComponent<Transform3D>(m_Entity2);

        float speed = 10.f;
        if (GPC::Inputs::IsKeyDown(Inputs::KeyCode::UP))
        {
            pEnt2Transform->LocalTransform.AddPosition(m_Clock.GetDeltaTime() * speed,0,0);
        }
        if (GPC::Inputs::IsKeyDown(Inputs::KeyCode::DOWN))
        {
            pEnt2Transform->LocalTransform.AddPosition(-m_Clock.GetDeltaTime() * speed,0,0);
        }
        if (GPC::Inputs::IsKeyDown(Inputs::KeyCode::LEFT))
        {
            pEnt2Transform->LocalTransform.AddPosition(0,0,-m_Clock.GetDeltaTime() * speed);
        }
        if (GPC::Inputs::IsKeyDown(Inputs::KeyCode::RIGHT))
        {
            pEnt2Transform->LocalTransform.AddPosition(0,0,m_Clock.GetDeltaTime() * speed);
        }

        if (GPC::Inputs::IsKeyDown(Inputs::KeyCode::Q))
        {
            Ray ray(
                mp_MainCameraTransform->LocalTransform.GetPosition(),
                -mp_MainCameraTransform->LocalTransform.GetForward()
            );

            RayHit hit;

            if (RayCast3D(hit, ray))
            {
                GetComponent<Transform3D>(m_RayContactVisualEntity)->LocalTransform.SetPosition(hit.ContactPoint);
            }
        }

        if (GetComponent<Collision3DComponent>(m_Entity2)->IsColliding())
            GetComponent<Render3DComponent>(m_Entity2)->RenderMaterial.Diffuse = m_RedTextureID;
        else
            GetComponent<Render3DComponent>(m_Entity2)->RenderMaterial.Diffuse = m_BlueTextureID;

    }

    void SceneCollision::OnSceneEndCalculation()
    {
        SceneDefault::OnSceneEndCalculation();
        float fps = 1.f / m_Clock.GetDeltaTime();

        //GPC_INFO << fps << " fps" << ENDL;
    }
}
