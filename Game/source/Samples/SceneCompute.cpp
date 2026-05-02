#include "Samples/SceneCompute.h"

#include <random>
#include <filesystem>

#include "Assets.h"
#include "FreeCamera.h"
#include "CameraSystem.h"
#include "ParticleSystem.h"
#include "Render3DSystem.h"
#include "Meta/MetaTypes.h"
#include "Tools/MetaGenerator.h"

namespace GPC
{
    ErrorType SceneCompute::OnAssetsLoad()
    {
            
        // Program Loading
        {
            GPC_ASSETS->AddShader("Default Vertex", "shader.vert", Shader::VERTEX);
            GPC_ASSETS->AddShader("Default Fragment", "shader.frag", Shader::FRAGMENT);

            GPC_ASSETS->AddGraphicProgram(
                "Default Program",
                GPC_SHADER("Default Vertex"),
                GPC_SHADER("Default Fragment"),
                CullMode::FRONT,
                PolygonMode::MODE_FILL,
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
                    },
                    GPC::DescriptorSetInformation {
                         .Name = "shadowSpotLightMapDepth",
                         .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                         .BindingSlot = 5,
                         .DescriptorCount = 1,
                         .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                         .Sampler_ = nullptr
                    },
                    GPC::DescriptorSetInformation
                    {
                        .Name = "LightTransform",
                        .Type = DescriptorType::UNIFORM_BUFFER,
                        .Usages = UNIFORM_BUFFER_BIT,
                        .BindingSlot = 6,
                        .DescriptorCount = 1,
                        .ElementCount = 1,
                        .ElementStride = sizeof(TransformLight),
                        .Flags = VK_SHADER_STAGE_VERTEX_BIT,
                        .Sampler_ = nullptr,
                    }
                },
                GRAPHICS_3D,
                mp_Window
            );


            // Texture Loading
            {
                GPC_ASSETS->AddTexture("carbon", "Resources/Materials/carbon1024.jpg");
                GPC_ASSETS->AddTexture("parquet", "Resources/Materials/parquet1024.jpg");

                GPC_ASSETS->AddTextureArray(
                    "arr",
                    1024, 1024,
                    {
                        GPC_TEXTURE("parquet"),
                        GPC_TEXTURE("carbon")
                    }
                );
            }

            auto program = GPC_GRAPHIC_PROGRAM("Default Program")->Program;

            program.InitializeWith({&GPC_TEXTURE_ARRAY("arr")->Array}, GPC_TEXTURE_ARRAY("arr")->TextureSampler);
            program.UseShadows(mp_Window);

        }

        GPC_ASSETS->AddShader("Particle Comp", "particles.comp", Shader::COMPUTE);
        GPC_ASSETS->AddParticleProgram(
            "Particle Program",
            {
                .Looping                = true,
                .MaxParticle            = 5000,
                .Mode                   = ParticleEmissionMode::BURST,
                .Shape                  = ParticleEmissionShape::SHPERE,
                .StartMinPosition       = glm::vec3(-1.0),
                .StartMaxPosition       = glm::vec3(1.0),
                .StartAfterSeconds      = 1.0f,
                .StartMinSpeed          = glm::vec3(-1.0),
                .StartMaxSpeed          = glm::vec3(1.0),
                .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                .StartMinSize           = glm::vec3(0.1),
                .StartMaxSize           = glm::vec3(0.5),
                .StartMinRotation       = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                .StartMaxRotation       = Transform::Euler_To_Quaternion(glm::pi<float>(), glm::pi<float>(), glm::pi<float>(), XYZ),
                .StartMaxColor          = Colors::ORANGE,
                .StartMinColor          = Colors::BLUE
            },
            GPC_SHADER("Particle Comp")
        );

        GPC_ASSETS->AddParticleProgram(
            "Particle OverTime Program",
            {
                .Looping                = true,
                .MaxParticle            = 5000,
                .Mode                   = ParticleEmissionMode::OVER_TIME,
                .Shape                  = ParticleEmissionShape::SHPERE,
                .StartMinPosition       = glm::vec3(-1.0),
                .StartMaxPosition       = glm::vec3(1.0),
                .StartAfterSeconds      = 1.0f,
                .LifeTimeMin            = 1.0f,
                .LifeTimeMax            = 10.0f,
                .StartMinSpeed          = glm::vec3(-1.0),
                .StartMaxSpeed          = glm::vec3(1.0),
                .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                .StartMinSize           = glm::vec3(0.1),
                .StartMaxSize           = glm::vec3(0.5),
                .StartMinRotation       = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                .StartMaxRotation       = Transform::Euler_To_Quaternion(glm::pi<float>(), glm::pi<float>(), glm::pi<float>(), XYZ),
                .StartMaxColor          = Colors::ORANGE,
                .StartMinColor          = Colors::BLUE
            },
            GPC_SHADER("Particle Comp")
        );

        mp_ComputeProgramBurst      = &GPC_COMPUTE_PROGRAM("Particle Program")->Program;
        mp_ComputeProgramOverTime   = &GPC_COMPUTE_PROGRAM("Particle OverTime Program")->Program;

        // Mesh Loading
        {
            GeometryData TRIANGLE_PRIMITIVE  ;
            GPC::Primitive::CreateTristan({1.f, 1.f}, &TRIANGLE_PRIMITIVE);
            GPC_ASSETS->AddMesh("Triangle", &TRIANGLE_PRIMITIVE);
            GeometryData CUBE_PRIMITIVE  ;
            GPC::Primitive::CreateCube({1.0f, 1.0f, 1.0f}, &CUBE_PRIMITIVE);
            GPC_ASSETS->AddMesh("Cube", &CUBE_PRIMITIVE);
            GeometryData SPHERE_PRIMITIVE  ;
            GPC::Primitive::CreateSphere(0.1f, 20, 20, &SPHERE_PRIMITIVE);
            GPC_ASSETS->AddMesh("Sphere", &SPHERE_PRIMITIVE);
        }

        return ErrorType::SUCCESS;
    }

    void SceneCompute::OnAssetsDestroy()
    {
    }

    ErrorType SceneCompute::OnCreate(SceneInformation &info)
    {
        auto camera         = CreateEntityAs3D();
        auto cTransform     = GetComponent<Transform3D>(camera);
        auto cCamera        = AddComponent<CameraComponent>(camera);
        auto cController    = AddBehavior<FreeCamera>(camera);
        cTransform->LocalTransform.AddPosition(0, 0, 5);

        auto base = CreateEntityAs3D();

        auto bMesh = AddComponent<Render3DComponent>(base);
        bMesh->pGeometry = GPC_MESH("Sphere");

        auto emitter = CreateEntityAs3D();
        m_BaseTransform     = GetComponent<Transform3D>(emitter);
        m_Emmiter = AddComponent<ParticleEmitter>(emitter);
        m_Emmiter->pGeometry = GPC_MESH("Triangle");
        m_Emmiter->pComputeProgram = mp_ComputeProgramOverTime;
        m_Emmiter->SetDuration(5, 0);
        m_Emmiter->SetLooping(true);
        m_Emmiter->AutoStart();

        auto emitter2 = CreateEntityAs3D();
        m_BaseTransform2     = GetComponent<Transform3D>(emitter2);
        m_Emmiter2 = AddComponent<ParticleEmitter>(emitter2);
        m_Emmiter2->pGeometry = GPC_MESH("Triangle");
        m_Emmiter2->pComputeProgram = mp_ComputeProgramOverTime;
        m_Emmiter2->SetDuration(5, 0);
        m_Emmiter2->SetLooping(true);
        m_Emmiter2->AutoStart();

        return ErrorType::SUCCESS;

    }

    void SceneCompute::OnSceneUpdate()
    {
        Scene::OnSceneUpdate();

        if (Inputs::IsKeyDown(Inputs::KeyCode::LEFT)) {
            m_BaseTransform->LocalTransform.AddPosition(0.1f, 0.0f, 0.0f);
        }
        if (Inputs::IsKeyDown(Inputs::KeyCode::RIGHT)) {
            m_BaseTransform->LocalTransform.AddPosition(-0.1f, 0.0f, 0.0f);
        }
        if (Inputs::IsKeyDown(Inputs::KeyCode::P)) {
            m_BaseTransform2->LocalTransform.AddPosition(0.1f, 0.0f, 0.0f);
        }
        if (Inputs::IsKeyDown(Inputs::KeyCode::M)) {
            m_BaseTransform2->LocalTransform.AddPosition(-0.1f, 0.0f, 0.0f);
        }

        if (Inputs::IsKeyPress(Inputs::KeyCode::O)) {
            m_Emmiter->pComputeProgram = mp_ComputeProgramOverTime;
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::B)) {
            m_Emmiter->pComputeProgram = mp_ComputeProgramBurst;
        }
    }

    void SceneCompute::OnDestroy()
    {

    }
}
