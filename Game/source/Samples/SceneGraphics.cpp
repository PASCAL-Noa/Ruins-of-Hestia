#include "../../include/Samples/SceneGraphics.h"

#include "Assets.h"
#include "FreeCamera.h"
#include "CameraSystem.h"
#include "LightingSystem.h"
#include "../../include/Expedition/PlayerController.h"
#include "Render3DSystem.h"
#include "Map Generation/TerrainGeneratorBehavior.h"

namespace GPC
{
    ErrorType SceneGraphics::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();

        // Batching Program
        {
            GPC_ASSETS->AddShader("Batching Vertex", "batching.vert", Shader::VERTEX);
            GPC_ASSETS->AddShader("Batching Fragment", "batching.frag", Shader::FRAGMENT);

            GPC_ASSETS->AddGraphicProgram(
                "Batching Program",
                GPC_SHADER("Batching Vertex"),
                GPC_SHADER("Batching Fragment"),
                CullMode::FRONT,
                PolygonMode::MODE_FILL,
                {
                    {
                        .BindingDescription = GPC::Vertex::GetBindingDescription(),
                        .AttributeDescriptions = GPC::Vertex::GetAttributeDescriptions()
                    },
                    {
                        .BindingDescription = GPC::Batched3DObject::GetBindingDescription(),
                        .AttributeDescriptions = GPC::Batched3DObject::GetAttributeDescriptions()
                    }
                },
                {
                    GPC::DescriptorSetInformation::ForCamera(0),
                    GPC::DescriptorSetInformation::ForMaterials(1),
                    GPC::DescriptorSetInformation::ForTextures(2),
                },
                GRAPHICS_3D,
                mp_Window
            );

            auto& program = GPC_GRAPHIC_PROGRAM("Batching Program")->Program;
            program.InitializeWith({&GPC_TEXTURE_ARRAY("1024x1024")->Array}, GPC_TEXTURE_ARRAY("1024x1024")->TextureSampler);


        }

        return ErrorType::SUCCESS;
    }

    void SceneGraphics::OnAssetsDestroy() {

    }

    ErrorType SceneGraphics::OnCreate(SceneInformation &info) {
        SceneDefault::OnCreate(info);

        auto cLight = AddComponent<LightingComponent>(mp_MainCamera->GetEntityID());
        cLight->pLightData = GPC_POINT_LIGHT("PointBlue");

        AddBehavior<GPC::FreeCamera>(mp_MainCamera->GetEntityID());

        // auto player = CreateEntityAs3D();
        // auto* pRenderer = AddComponent<GPC::Render3DComponent>(player);
        // auto* pTransform = GetComponent<GPC::Transform3D>(player);
        // auto* pLight = AddComponent<GPC::LightingComponent>(player);

        // pRenderer->pGeometry = GPC_MESH("Cube");
        // // pRenderer->pGraphicProgram = &GPC_GRAPHIC_PROGRAM("Default Program")->Program; // TODO New line <- Check it out
        // pRenderer->RenderMaterial.Diffuse = GPC_TEXTURE_ID("1024x1024", "Debug");
        // pTransform->LocalTransform.SetPosition(0, 1, 0);
        // pTransform->LocalTransform.SetScale(1, 1, 1);
        // pLight->pLightData = GPC_POINT_LIGHT("PointWhite");

        // mp_MainCameraTransform->LocalTransform.SetPosition(pTransform->LocalTransform.GetPosition() + glm::vec3{-3, 8, -3});

        auto    batchedTrees = CreateEntityAs3D();
        auto*   pBatching = AddComponent<GPC::Batching3DComponent>(batchedTrees);

        pBatching->Batched3DObjects.resize(10);

        pBatching->Batched3DObjects[0] = {
            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f},
            Color(125, 125, 125, 255)
        };

        pBatching->Batched3DObjects[1] = {
            glm::vec4(2.0f, 0.0f, 0.0f, 1.0f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f},
            Color(125, 125, 125, 255)
        };

        pBatching->Batched3DObjects[2] = {
            glm::vec4(3.0f, 0.0f, 0.0f, 1.0f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f},
            Color(125, 125, 125, 255)
        };

        pBatching->pGeometry = GPC_MESH("Cube");
        pBatching->pGraphicProgram = &GPC_GRAPHIC_PROGRAM("Batching Program")->Program;
        pBatching->SetTexture("1024x1024", "DefaultNormal");


        return ErrorType::SUCCESS;
    }

    void SceneGraphics::OnDestroy() {

    }

} // GPC