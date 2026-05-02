#include "Scenes/SceneDefault.h"
#include "Meta/MetaTypes.h"
#include <cmath>
#include <filesystem>

#include "LightingSystem.h"
#include "Render3DSystem.h"
#include "Tools/MetaGenerator.h"
#include "UI/UiBuilder.h"
#include "UI/UiCanvas.h"
#include "UI/UiText.h"
#include "UI/UiTheme.h"
#include "UiConstants.h"

namespace GPC
{
    void SceneDefault::LoadTextureAndArrayDefault(const std::string &name, const std::string &filepath) {
        GPC_ASSETS->AddTexture("_" + name, filepath);
        GPC_ASSETS->AddTextureArray(name, 1024, 1024, {
            GPC_TEXTURE("DefaultNormal"),
            GPC_TEXTURE("DefaultHeight"),
            GPC_TEXTURE("_" + name),
        });
    }

    void SceneDefault::LoadDefaultGraphicShader(const std::string &name, const std::string &vertexPath,
        const std::string &fragmentPath, const std::vector<std::string> &arrays) {

         GPC_ASSETS->AddShader(name + " Vertex", vertexPath, Shader::VERTEX);
        GPC_ASSETS->AddShader( name + " Fragment", fragmentPath, Shader::FRAGMENT);

        GPC_ASSETS->AddGraphicProgram(
            name,
            GPC_SHADER(name + " Vertex"),
            GPC_SHADER(name + " Fragment"),
            CullMode::FRONT,
            PolygonMode::MODE_FILL,
            {
                {
                    .BindingDescription = GPC::Vertex::GetBindingDescription(),
                    .AttributeDescriptions = GPC::Vertex::GetAttributeDescriptions()
                }
            },
            {
                GPC::DescriptorSetInformation::ForCamera(0),
                GPC::DescriptorSetInformation::ForTransform(1),
                GPC::DescriptorSetInformation::ForMaterials(2),
                GPC::DescriptorSetInformation::ForLights(3),
                GPC::DescriptorSetInformation {
                     .Name = "images",
                     .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                     .BindingSlot = 4,
                     .DescriptorCount = 50,
                     .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                     .Sampler_ = nullptr
                },
                GPC::DescriptorSetInformation {
                    .Name = "LightTransform",
                    .Type = DescriptorType::UNIFORM_BUFFER,
                    .Usages = UNIFORM_BUFFER_BIT,
                    .BindingSlot = 5,
                    .DescriptorCount = 1,
                    .ElementCount = 1,
                    .ElementStride = sizeof(TransformLight),
                    .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .Sampler_ = nullptr
                },
                GPC::DescriptorSetInformation {
                     .Name = "shadowSpotLightMapDepth",
                     .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                     .BindingSlot = 6,
                     .DescriptorCount = 1,
                     .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                     .Sampler_ = nullptr
                },
                GPC::DescriptorSetInformation {
                     .Name = "shadowPointLightMapDepth",
                     .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                     .BindingSlot = 7,
                     .DescriptorCount = 1,
                     .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                     .Sampler_ = nullptr,
                },
                GPC::DescriptorSetInformation {
                     .Name = "shadowDirLightMapDepth",
                     .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                     .BindingSlot = 8,
                     .DescriptorCount = 1,
                     .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                     .Sampler_ = nullptr,
                }
            },
            GRAPHICS_3D,
            mp_Window
        );

        if (arrays.empty()) return;

        auto& program = GPC_GRAPHIC_PROGRAM(name)->Program;

        std::vector<Texture*> pArrays{};
        Sampler* pSampler = nullptr;

        for (auto array : arrays) {
            auto pArray = GPC_TEXTURE_ARRAY(array);
            if (pArray == nullptr) {
                GPC_WARNING << "Fail to get texture array (" << array << ") for graphic program " << name << ENDL;
                continue;
            }
            pArrays.push_back(&pArray->Array);
            pSampler = &pArray->TextureSampler;
        }

        program.InitializeWith(
            pArrays,
            *pSampler
        );

        program.UseShadows(mp_Window);
    }

    void SceneDefault::LoadTextureArray(const std::string &name, float width, float height,
        const std::vector<std::pair<std::string, std::string>> &textureNamePaths) {

        std::vector<TextureAsset*> pTextureAssetsLoaded{};
        for (auto texture : textureNamePaths) {
            auto tex = GPC_ASSETS->AddTexture(texture.first, texture.second);
            if (tex == nullptr) {
                GPC_WARNING << "Fail to load texture : " << texture.second << ENDL;
                continue;
            }
            pTextureAssetsLoaded.push_back(GPC_TEXTURE(texture.first));
        }

        GPC_ASSETS->AddTextureArray(
            name,
            width, height,
            pTextureAssetsLoaded
        );
    }


    ErrorType SceneDefault::OnAssetsLoad() {


        // Texture
        {
            LoadTextureArray(
                "1024x1024",
                1024, 1024,
                {
                    { "DefaultNormal", "Resources/Materials/DefaultNormal.png" },
                    { "DefaultHeight", "Resources/Materials/DefaultHeight.png" },
                    { "Perlin", "Resources/Materials/PerlinNoise.jpg" },
                    { "Blue", "Resources/Materials/blue.jpg" },
                    { "Red", "Resources/Materials/red.jpg" },
                    { "Grey", "Resources/Materials/grey.jpg" },
                    { "White", "Resources/Materials/white.jpg" },
                    { "Grass", "Resources/Materials/Seamless/Orange/0.png"  },
                    { "Grass2", "Resources/Materials/Seamless/Pink/0.png" },

                    { "Seamless1", "Resources/Materials/Seamless/0.png" },
                    { "Seamless2", "Resources/Materials/Seamless/1.png" },
                    { "Seamless3", "Resources/Materials/Seamless/2.png" },
                    { "Seamless4", "Resources/Materials/Seamless/3.png" },
                    { "Seamless5", "Resources/Materials/Seamless/4.png" },
                    { "Seamless6", "Resources/Materials/Seamless/5.png" },
                    { "Seamless7", "Resources/Materials/Seamless/6.png" },
                    { "Seamless8", "Resources/Materials/Seamless/7.png" },
                    { "Stone", "Resources/Materials/tile mur.PNG" },
                }
            );

            std::vector<std::pair<std::string, std::string>> ResourcesAndDecor;
            for (uint32_t i_tronc = 0; i_tronc < TRONC_COUNT; ++i_tronc) {
                ResourcesAndDecor.emplace_back("Tronc" + std::to_string(i_tronc), "Resources/Materials/Trees/Tronc/" + std::to_string(i_tronc) + ".png");
            }
            for (uint32_t i_leavesColor = 0; i_leavesColor < LEAVES_COLOR_COUNT; ++i_leavesColor) {
                for (uint32_t i_leaves = 0; i_leaves < TRONC_COUNT; ++i_leaves) {
                    ResourcesAndDecor.emplace_back("Leaves" + std::to_string(i_leavesColor) + "_" + std::to_string(i_leaves), "Resources/Materials/Trees/Leaves" + std::to_string(i_leavesColor) + "/" + std::to_string(i_leaves) + ".png");
                }
            }
            for (uint32_t i_caillou = 0; i_caillou < CAILLOUX_COUNT; ++i_caillou) {
                ResourcesAndDecor.emplace_back("Cailloux" + std::to_string(i_caillou), "Resources/Materials/Cailloux/Pierre/" + std::to_string(i_caillou) + ".png");
            }
            for (uint32_t i_gem = 0; i_gem < GEM_COUNT; ++i_gem) {
                ResourcesAndDecor.emplace_back("BlueGem" + std::to_string(i_gem), "Resources/Materials/Cailloux/Blue/" + std::to_string(i_gem) + ".png");
            }
            for (uint32_t i_gem = 0; i_gem < GEM_COUNT; ++i_gem) {
                ResourcesAndDecor.emplace_back("YellowGem" + std::to_string(i_gem), "Resources/Materials/Cailloux/Yellow/" + std::to_string(i_gem) + ".png");
            }
            for (uint32_t i_gem = 0; i_gem < GEM_COUNT; ++i_gem) {
                ResourcesAndDecor.emplace_back("GreenGem" + std::to_string(i_gem), "Resources/Materials/Cailloux/Green/" + std::to_string(i_gem) + ".png");
            }
            for (uint32_t i_gem = 0; i_gem < GEM_COUNT; ++i_gem) {
                ResourcesAndDecor.emplace_back("RedGem" + std::to_string(i_gem), "Resources/Materials/Cailloux/Red/" + std::to_string(i_gem) + ".png");
            }
            for (uint32_t i_gem = 0; i_gem < GEM_COUNT; ++i_gem) {
                ResourcesAndDecor.emplace_back("PurpleGem" + std::to_string(i_gem), "Resources/Materials/Cailloux/Purple/" + std::to_string(i_gem) + ".png");
            }
            for (uint32_t i_gem = 0; i_gem < GEM_COUNT; ++i_gem) {
                ResourcesAndDecor.emplace_back("OrangeGem" + std::to_string(i_gem), "Resources/Materials/Cailloux/Orange/" + std::to_string(i_gem) + ".png");
            }
            LoadTextureArray(
                "ResourceAndDecor",
                1024, 1024,
                ResourcesAndDecor
            );

            std::vector<std::pair<std::string, std::string>> Props;
            for (uint32_t i_prop = 0; i_prop < PROP_COUNT; ++i_prop) {
                Props.emplace_back("Bush" + std::to_string(i_prop), "Resources/Materials/Props/" + std::to_string(i_prop) + ".png");
            }
            LoadTextureArray(
                "Props",
                600, 300,
                Props
            );

            Props.clear();
            for (uint32_t i_prop = 0; i_prop < PROP_LOAD_COUNT; ++i_prop) {
                Props.emplace_back("Bush" + std::to_string(i_prop), "Resources/Materials/Props/" + std::to_string(i_prop) + ".png");
            }

            LoadTextureArray(
                "PropsVillage",
                600, 300,
                Props
            );

            LoadTextureArray(
                "2048x2048",
                2048, 2048,
                {
                    { "CaliTexture", "Resources/Materials/CaliTexture.png" },
                    { "SileneTexture", "Resources/Materials/MTL_Silene_Base_color.png" },
                    { "EnnemiTexture", "Resources/Materials/EnnemiTexture.png" },
                    { "BossTexture", "Resources/Materials/BossTexture.png" },
                    { "IrisTexture", "Resources/Materials/IrisTexture.png" },
                    { "Water", "Resources/Materials/eau.PNG" },
                    { "Ground", "Resources/Materials/TILE VILLAGE.PNG" },
                }
            );

            LoadTextureArray(
                "1920x1080",
                1920, 1080,
                {
                    { "TransiFeuilleLeft", "Resources/Materials/TransiFeuille/Left.png" },
                    { "TransiFeuilleRight", "Resources/Materials/TransiFeuille/Right.png" },
                }
            );

            LoadTextureArray(
                "1172x745",
                1172, 745,
                {
                    { "CampFire", "Resources/Materials/CAMPFIRE2.png" }
                }
            );

            LoadTextureArray(
                "619x642",
                619, 642,
                {
                    { "Fleche", "Resources/Materials/fleche.png" },
                    {"LogoStudio", "Resources/Images/LogoStudio.png"}
                }
            );

            LoadTextureArray(
                "512x512",
                512, 512,
                {
                    { "Wood", "Resources/Materials/Wood.png" },
                    { "BG_MAP", "Resources/Materials/bg_map_ui.png" },
                    { "PLAYER_MAP", "Resources/Materials/player_map_ui.png" },
                }
            );


            LoadTextureArray(
                "100x100",
                100, 100,
                {
                    { "Particle_Leaves0", "Resources/Materials/Particles/f0.png" },
                    { "Particle_Leaves1", "Resources/Materials/Particles/f1.png" },
                    { "Particle_Leaves2", "Resources/Materials/Particles/f2.png" },
                    { "Particle_Leaves3", "Resources/Materials/Particles/f3.png" },
                    { "Particle_Leaves4", "Resources/Materials/Particles/f4.png" },
                    { "Particle_Leaves5", "Resources/Materials/Particles/f5.png" },
                    { "Particle_Leaves6", "Resources/Materials/Particles/f6.png" },
                    { "Particle_Leaves7", "Resources/Materials/Particles/f7.png" },
                    { "Particle_Leaves8", "Resources/Materials/Particles/f8.png" },
                    { "Particle_Leaves9", "Resources/Materials/Particles/f9.png" },
                    { "Pickup_Wood", "Resources/UI/EXPEDITION/100x100/pickup_wood.png" },
                    { "Pickup_Stone", "Resources/UI/EXPEDITION/100x100/pickup_stone.png" },
                    { "Pickup_Red", "Resources/UI/EXPEDITION/100x100/pickup_red.png" },
                    { "Pickup_Blue", "Resources/UI/EXPEDITION/100x100/pickup_blue.png" },
                    { "Pickup_Yellow", "Resources/UI/EXPEDITION/100x100/pickup_yellow.png" },
                    { "Pickup_Green", "Resources/UI/EXPEDITION/100x100/pickup_green.png" },
                    { "Pickup_Purple", "Resources/UI/EXPEDITION/100x100/pickup_purple.png" },
                    { "Pickup_Orange", "Resources/UI/EXPEDITION/100x100/pickup_orange.png" },
                }
            );
            LoadTextureArray(
                "1220x1683",
                1220, 1683,
                {
                    { "Building_forge",        "Resources/Materials/Village/forge.png" },
                    { "Building_forge_active", "Resources/Materials/Village/forge_active.png" },
                    { "Building_forge_highlight","Resources/Materials/Village/forge-highlight.png" },
                    { "Building_farm",         "Resources/Materials/Village/farm.png" },
                    { "Building_farm_active",  "Resources/Materials/Village/farm_active.png" },
                    { "Building_farm_highlight","Resources/Materials/Village/farm-highlight.png" },
                    { "Building_house",        "Resources/Materials/Village/house.png" },
                    { "Building_house_active", "Resources/Materials/Village/house_active.png" },
                    { "Building_house_highlight","Resources/Materials/Village/house-highlight.png" },
                    { "Building_LifeTree_0",   "Resources/Materials/Village/lifetree1.png" },
                    { "Building_LifeTree_0_highlight","Resources/Materials/Village/lifetree1-highlight.png" },
                    { "Building_LifeTree_1",   "Resources/Materials/Village/life_tree_1.png" },
                    { "Building_LifeTree_2",   "Resources/Materials/Village/life_tree_2.png" },
                }
            );

            LoadTextureArray(
                "2067x2067",
                2067, 2067,
                {
                    { "Grid", "Resources/Materials/grille.png" },
                }
            );

            LoadTextureArray(
                "16x16_ui",
                16, 16,
                {
                    { "Ui_Black", "Resources/Images/Black.png" },
                }
            );

            LoadTextureArray(
                "1411x418",
                1411, 418,
                {
                    { "Bg_Dialogue", "Resources/UI/Dialogue/ui_dialogue_text.png" },
                }
            );

            LoadTextureArray(
                "283x156",
                283, 156,
                {
                    { "Bg_Dialogue_Name", "Resources/UI/Dialogue/ui_dialogue_name.png" },
                }
            );

            LoadTextureArray(
                "317x642",
                317, 642,
                {
                    { "Cali_Face", "Resources/UI/Dialogue/cali_face.png" },
                }
            );

            LoadTextureArray(
                "303x642",
                303, 642,
                {
                    { "Iris_Face", "Resources/UI/Dialogue/iris_face.png" },
                }
            );

            LoadTextureArray(
                "212x642",
                212, 642,
                {
                    { "Silene_Face", "Resources/UI/Dialogue/silene_face.png" },
                }
            );

            LoadTextureArray(
                "1443x998",
                1443, 998,
                {
                    { "Settings_Background", "Resources/UI/SETTINGS/1443x998/ui_settings_panel.png" },
                }
            );

            LoadTextureArray(
                "44x43",
                44, 43,
                {
                    { "Settings_Plus",      "Resources/UI/SETTINGS/44x43/ui_settings_plus.png" },
                    { "Settings_Minus",     "Resources/UI/SETTINGS/44x43/ui_settings_minus.png" },
                    { "Settings_Check",     "Resources/UI/SETTINGS/44x43/ui_settings_check.png" },
                    { "Settings_CheckBox",  "Resources/UI/SETTINGS/44x43/ui_settings_checkbox.png" },
                }
            );

            LoadTextureArray(
                "1368x1920",
                1368, 1920,
                {
                { "Village_Background", "Resources/Materials/MIDDLE_TREE.png" },
                { "Village_Background_End", "Resources/Materials/END_TREE.png" },
                { "Village_Background_End_Flipped", "Resources/Materials/START_TREE.PNG" },
                }
            );

            LoadTextureArray(
                "1193x139",
                1193, 139,
                {
                    { "Boss_Health_BG", "Resources/UI/HUD/1193x139/ui_varos_hud_empty.png" },
                    { "Boss_Health", "Resources/UI/HUD/1193x139/ui_varos_hp.png" },
                }
            );
        }

        // Program
        {
            LoadDefaultGraphicShader(
                "Default Program",
                "shader.vert",
                "shader.frag",
                {
                    "1024x1024",
                    "619x642",
                    "1220x1683",
                    "2067x2067",
                    "2048x2048",
                    "1172x745",
                    "1368x1920",
                    "ResourceAndDecor",
                    "PropsVillage"
                }
            );

            GPC_ASSETS->AddShader("Props Vertex", "prop.vert", Shader::VERTEX);
            GPC_ASSETS->AddShader("Props Fragment", "prop.frag", Shader::FRAGMENT);

            for (uint32_t i = 0; i < PROP_COUNT; ++i) {
                GPC_ASSETS->AddGraphicProgram(
                    "Props" + std::to_string(i) + " Program",
                    GPC_SHADER("Props Vertex"),
                    GPC_SHADER("Props Fragment"),
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

                auto& program = GPC_GRAPHIC_PROGRAM("Props" + std::to_string(i) + " Program")->Program;
                program.InitializeWith({&GPC_TEXTURE_ARRAY("Props")->Array}, GPC_TEXTURE_ARRAY("1024x1024")->TextureSampler);
            }


            LoadDefaultGraphicShader(
                "Unlit Program",
                "unlit.vert",
                "unlit.frag",
                {
                    "1920x1080",
                }
            );
        }

        {
            GPC_ASSETS->AddShader("Compute Shader Particle", "particles.comp", Shader::Type::COMPUTE);

            GPC_ASSETS->AddParticleProgram(
               "Wood Particle Program",
               {
                   .Looping                = false,
                   .MaxParticle            = 12,
                   .Mode                   = ParticleEmissionMode::BURST,
                   .Shape                  = ParticleEmissionShape::SHPERE,
                   .StartMinPosition       = glm::vec3(-1.0),
                   .StartMaxPosition       = glm::vec3(1.0),
                   .StartAfterSeconds      = 0.0f,
                   .StartMinSpeed          = glm::vec3(-7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSpeed          = glm::vec3(7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                   .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                   .StartMinSize           = glm::vec3(0.02 * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSize           = glm::vec3(0.1 * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxColor          = Colors::DARK_BLUE,
                   .StartMinColor          = Colors::FUCHSIA,
                   .Duration = 1.0f
               },
               GPC_SHADER("Compute Shader Particle")
           );

            GPC_ASSETS->AddParticleProgram(
               "Stone Particle Program",
               {
                   .Looping                = false,
                   .MaxParticle            = 12,
                   .Mode                   = ParticleEmissionMode::BURST,
                   .Shape                  = ParticleEmissionShape::SHPERE,
                   .StartMinPosition       = glm::vec3(-1.0),
                   .StartMaxPosition       = glm::vec3(1.0),
                   .StartAfterSeconds      = 0.0f,
                   .StartMinSpeed          = glm::vec3(-7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSpeed          = glm::vec3(7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                   .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                   .StartMinSize           = glm::vec3(0.02 * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSize           = glm::vec3(0.1 * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxColor          = Colors::GRAY,
                   .StartMinColor          = Colors::DARK_GRAY,
                   .Duration = 1.0f
               },
               GPC_SHADER("Compute Shader Particle")
            );

            GPC_ASSETS->AddParticleProgram(
               "BlueGem Particle Program",
               {
                   .Looping                = false,
                   .MaxParticle            = 12,
                   .Mode                   = ParticleEmissionMode::BURST,
                   .Shape                  = ParticleEmissionShape::SHPERE,
                   .StartMinPosition       = glm::vec3(-1.0),
                   .StartMaxPosition       = glm::vec3(1.0),
                   .StartAfterSeconds      = 0.0f,
                   .StartMinSpeed          = glm::vec3(-7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSpeed          = glm::vec3(7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                   .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                   .StartMinSize           = glm::vec3(0.02 * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSize           = glm::vec3(0.1 * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxColor          = Colors::BLUE,
                   .StartMinColor          = Colors::DARK_BLUE,
                   .Duration = 1.0f
               },
               GPC_SHADER("Compute Shader Particle")
            );

            GPC_ASSETS->AddParticleProgram(
               "RedGem Particle Program",
               {
                   .Looping                = false,
                   .MaxParticle            = 12,
                   .Mode                   = ParticleEmissionMode::BURST,
                   .Shape                  = ParticleEmissionShape::SHPERE,
                   .StartMinPosition       = glm::vec3(-1.0),
                   .StartMaxPosition       = glm::vec3(1.0),
                   .StartAfterSeconds      = 0.0f,
                   .StartMinSpeed          = glm::vec3(-7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSpeed          = glm::vec3(7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                   .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                   .StartMinSize           = glm::vec3(0.02 * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSize           = glm::vec3(0.1 * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxColor          = Colors::RED,
                   .StartMinColor          = Colors::DARK_RED,
                   .Duration = 1.0f
               },
               GPC_SHADER("Compute Shader Particle")
            );

            GPC_ASSETS->AddParticleProgram(
               "YellowGem Particle Program",
               {
                   .Looping                = false,
                   .MaxParticle            = 12,
                   .Mode                   = ParticleEmissionMode::BURST,
                   .Shape                  = ParticleEmissionShape::SHPERE,
                   .StartMinPosition       = glm::vec3(-1.0),
                   .StartMaxPosition       = glm::vec3(1.0),
                   .StartAfterSeconds      = 0.0f,
                   .StartMinSpeed          = glm::vec3(-7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSpeed          = glm::vec3(7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                   .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                   .StartMinSize           = glm::vec3(0.02 * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSize           = glm::vec3(0.1 * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxColor          = Colors::YELLOW,
                   .StartMinColor          = Colors::YELLOW_GREEN,
                   .Duration = 1.0f
               },
               GPC_SHADER("Compute Shader Particle")
            );

            GPC_ASSETS->AddParticleProgram(
               "GreenGem Particle Program",
               {
                   .Looping                = false,
                   .MaxParticle            = 12,
                   .Mode                   = ParticleEmissionMode::BURST,
                   .Shape                  = ParticleEmissionShape::SHPERE,
                   .StartMinPosition       = glm::vec3(-1.0),
                   .StartMaxPosition       = glm::vec3(1.0),
                   .StartAfterSeconds      = 0.0f,
                   .StartMinSpeed          = glm::vec3(-7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSpeed          = glm::vec3(7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                   .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                   .StartMinSize           = glm::vec3(0.02 * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSize           = glm::vec3(0.1 * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxColor          = Colors::GREEN,
                   .StartMinColor          = Colors::DARK_GREEN,
                   .Duration = 1.0f
               },
               GPC_SHADER("Compute Shader Particle")
            );

            GPC_ASSETS->AddParticleProgram(
               "PurpleGem Particle Program",
               {
                   .Looping                = false,
                   .MaxParticle            = 12,
                   .Mode                   = ParticleEmissionMode::BURST,
                   .Shape                  = ParticleEmissionShape::SHPERE,
                   .StartMinPosition       = glm::vec3(-1.0),
                   .StartMaxPosition       = glm::vec3(1.0),
                   .StartAfterSeconds      = 0.0f,
                   .StartMinSpeed          = glm::vec3(-7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSpeed          = glm::vec3(7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                   .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                   .StartMinSize           = glm::vec3(0.02 * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSize           = glm::vec3(0.1 * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxColor          = Colors::PURPLE,
                   .StartMinColor          = Colors::DARK_PURPLE,
                   .Duration = 1.0f
               },
               GPC_SHADER("Compute Shader Particle")
            );

            GPC_ASSETS->AddParticleProgram(
               "OrangeGem Particle Program",
               {
                   .Looping                = false,
                   .MaxParticle            = 12,
                   .Mode                   = ParticleEmissionMode::BURST,
                   .Shape                  = ParticleEmissionShape::SHPERE,
                   .StartMinPosition       = glm::vec3(-1.0),
                   .StartMaxPosition       = glm::vec3(1.0),
                   .StartAfterSeconds      = 0.0f,
                   .StartMinSpeed          = glm::vec3(-7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSpeed          = glm::vec3(7.5f * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                   .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                   .StartMinSize           = glm::vec3(0.02 * GENERAL_SCALE_EXPEDITION),
                   .StartMaxSize           = glm::vec3(0.1 * GENERAL_SCALE_EXPEDITION),
                   .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                   .StartMaxColor          = Colors::ORANGE,
                   .StartMinColor          = Colors::DARK_ORANGE,
                   .Duration = 1.0f
               },
               GPC_SHADER("Compute Shader Particle")
            );

        }


        // Mesh
        {
            GeometryData CUBE_PRIMITIVE  ;
            GeometryData SPHERE_PRIMITIVE;
            GeometryData PLANE_PRIMITIVE ;
            GeometryData PLANE_INVERTED_PRIMITIVE ;
            GeometryData PLANE_2x2_PRIMITIVE ;
            GeometryData PLANE_4x4_PRIMITIVE ;
            GeometryData PLANE_20x20_PRIMITIVE ;
            GeometryData DOUBLE_SIDED_PLANE_PRIMITIVE ;
            GeometryData RECTANGLE_PRIMITIVE ;

            GPC::Primitive::CreateCube({1.f, 1.f, 1.f}, &CUBE_PRIMITIVE);
            GPC::Primitive::CreateSphere(0.5f, 30, 30, &SPHERE_PRIMITIVE);
            GPC::Primitive::CreatePlane(glm::vec2(1.0f, 1.0f), 1, &PLANE_PRIMITIVE);
            GPC::Primitive::CreatePlane(glm::vec2(1.0f, 1.0f), 2, &PLANE_2x2_PRIMITIVE);
            GPC::Primitive::CreatePlane(glm::vec2(1.0f, 1.0f), 4, &PLANE_4x4_PRIMITIVE);
            GPC::Primitive::CreatePlane(glm::vec2(1.0f, 1.0f), 20, &PLANE_20x20_PRIMITIVE);
            GPC::Primitive::CreatePlaneDS(glm::vec2(1.0f, 1.0f), 1, &DOUBLE_SIDED_PLANE_PRIMITIVE);
            GPC::Primitive::CreateRectangle(glm::vec2(1.0f, 1.0f), &RECTANGLE_PRIMITIVE);

            GPC_ASSETS->AddMesh("Cube", &CUBE_PRIMITIVE);
            GPC_ASSETS->AddMesh("Sphere", &SPHERE_PRIMITIVE);
            GPC_ASSETS->AddMesh("PlaneDS", &DOUBLE_SIDED_PLANE_PRIMITIVE);
            GPC_ASSETS->AddMesh("Plane", &PLANE_PRIMITIVE);
            GPC_ASSETS->AddMesh("Plane2x2", &PLANE_2x2_PRIMITIVE);
            GPC_ASSETS->AddMesh("Plane4x4", &PLANE_4x4_PRIMITIVE);
            GPC_ASSETS->AddMesh("Plane20x20", &PLANE_20x20_PRIMITIVE);
            GPC_ASSETS->AddMesh("Rectangle", &RECTANGLE_PRIMITIVE);

        }

        // Light
        {
            LightDescriptor red_light{};
            red_light.EmissionColor = { 255, 0, 0, 255 };
            red_light.Type = Light::LightType::POINT;
            red_light.Intensity = 100000.0f;

            LightDescriptor green_light{};
            green_light.EmissionColor = { 0, 255, 0, 255 };
            green_light.Type = Light::LightType::POINT;
            green_light.Intensity = 100000.0f;

            LightDescriptor blue_light{};
            blue_light.EmissionColor = { 0, 0, 255, 255 };
            blue_light.Type = Light::LightType::POINT;
            blue_light.Intensity = 100000.0f;

            LightDescriptor white_light{};
            white_light.EmissionColor = { 255, 255, 255, 255 };
            white_light.Type = Light::LightType::POINT;
            white_light.Intensity = 1000.0f;

            LightDescriptor white_light_directional{};
            white_light_directional.EmissionColor = { 255, 255, 255, 255 };
            white_light_directional.Type = Light::LightType::DIRECTIONAL;
            white_light_directional.Direction = {0.0f, -1.0f, 0.0f, 1.0f};
            white_light_directional.Intensity = 20.0f;

            LightDescriptor white_light_spot{};
            white_light_spot.EmissionColor = { 255, 255, 255, 255 };
            white_light_spot.Type = Light::LightType::SPOT;
            white_light_spot.Intensity = 40.0f;

            LightDescriptor blue_light_spot{};
            blue_light_spot.EmissionColor = { 0, 0, 255, 255 };
            blue_light_spot.Type = Light::LightType::SPOT;
            blue_light_spot.Intensity = 40.0f;

            LightDescriptor red_light_spot{};
            red_light_spot.EmissionColor = { 255, 0, 0, 255 };
            red_light_spot.Type = Light::LightType::SPOT;
            red_light_spot.Intensity = 40.0f;

            LightDescriptor green_light_spot{};
            green_light_spot.EmissionColor = { 0, 255, 0, 255 };
            green_light_spot.Type = Light::LightType::SPOT;
            green_light_spot.Intensity = 40.0f;

            LightDescriptor sunset_light{};
            sunset_light.EmissionColor = { 246, 125, 49, 255 };
            sunset_light.Type = Light::LightType::DIRECTIONAL;
            sunset_light.Intensity = 30.0f;

            LightDescriptor moon_light{};
            moon_light.EmissionColor = { 9, 50, 180, 255 };
            moon_light.Type = Light::LightType::DIRECTIONAL;
            moon_light.Intensity = 15.0f;

            GPC_ASSETS->AddLight("PointRed",    red_light);
            GPC_ASSETS->AddLight("PointGreen",  green_light);
            GPC_ASSETS->AddLight("PointBlue",   blue_light);
            GPC_ASSETS->AddLight("PointWhite",  white_light);
            GPC_ASSETS->AddLight("DirWhite",  white_light_directional);
            GPC_ASSETS->AddLight("SpotBlue",  blue_light_spot);
            GPC_ASSETS->AddLight("SpotWhite",  white_light_spot);
            GPC_ASSETS->AddLight("SpotRed",  red_light_spot);
            GPC_ASSETS->AddLight("SpotGreen",  green_light_spot);
            GPC_ASSETS->AddLight("DirSunset",  sunset_light);
            GPC_ASSETS->AddLight("DirMoon",  moon_light);
        }

        // Font
        {
            GPC_ASSETS->AddFont("Eczar", "Resources/Fonts/Ruinsofhestia-Regular.otf", 108);
        }

        // Audio
        {
            // TUTO
            GPC_ASSETS->AddAudio("Ecran_titre", "Resources/Audio/Music/Ruins_of_Hestia_ecran_titre.wav");
            GPC_ASSETS->AddAudio("ROH_explore", "Resources/Audio/Music/Ruins_of_hestia_explore.wav");
            GPC_ASSETS->AddAudio("ROH_fight",           "Resources/Audio/Music/Ruins_of_hestia_fight.wav");
            GPC_ASSETS->AddAudio("ROH_village_intro",   "Resources/Audio/Music/ROHvillageintro.wav");
            GPC_ASSETS->AddAudio("ROH_village",         "Resources/Audio/Music/ROHvillageloop.wav");
            GPC_ASSETS->AddAudio("Nature",              "Resources/Audio/Music/nature.wav");
            GPC_ASSETS->AddAudio("Nature_only_wind", "Resources/Audio/Music/naturewind.wav");
            GPC_ASSETS->AddAudio("Nature_only_birds", "Resources/Audio/Music/naturebird.wav");

            GPC_ASSETS->AddAudio("Slide", "Resources/Audio/SFX/temp_dash.wav");
            GPC_ASSETS->AddAudio("Hit", "Resources/Audio/SFX/zemmour-tousse.wav");
            GPC_ASSETS->AddAudio("Shoot", "Resources/Audio/SFX/sfx.wav");
            GPC_ASSETS->AddAudio("Mining", "Resources/Audio/SFX/mining_hit.wav");
            GPC_ASSETS->AddAudio("Chip1", "Resources/Audio/SFX/chip1.wav");
            GPC_ASSETS->AddAudio("Chip2", "Resources/Audio/SFX/chip2.wav");
            GPC_ASSETS->AddAudio("Chip3", "Resources/Audio/SFX/chip3.wav");

            // EXPEDITION
            GPC_ASSETS->AddAudio("Defeat", "Resources/Audio/SFX/sfx_defeat.wav");
            GPC_ASSETS->AddAudio("Victory", "Resources/Audio/SFX/sfx_victory.wav");
            GPC_ASSETS->AddAudio("ROH_explore", "Resources/Audio/Music/Ruins_of_hestia_explore.wav");
            GPC_ASSETS->AddAudio("ROH_fight", "Resources/Audio/Music/Ruins_of_hestia_fight.wav");

            // EXPEDITION CALI
            GPC_ASSETS->AddAudio("Chemin1", "Resources/Audio/SFX/bruits de pas/chemin1.wav");
            GPC_ASSETS->AddAudio("Chemin2", "Resources/Audio/SFX/bruits de pas/chemin2.wav");
            GPC_ASSETS->AddAudio("Chemin3", "Resources/Audio/SFX/bruits de pas/chemin3.wav");
            GPC_ASSETS->AddAudio("Chemin4", "Resources/Audio/SFX/bruits de pas/chemin4.wav");
            GPC_ASSETS->AddAudio("Chemin5", "Resources/Audio/SFX/bruits de pas/chemin5.wav");
            GPC_ASSETS->AddAudio("Chemin6", "Resources/Audio/SFX/bruits de pas/chemin6.wav");
            GPC_ASSETS->AddAudio("Slide1", "Resources/Audio/SFX/EXPEDITION/dash_1.wav");
            GPC_ASSETS->AddAudio("Slide2", "Resources/Audio/SFX/EXPEDITION/dash_2en1.wav");
            GPC_ASSETS->AddAudio("Slide3", "Resources/Audio/SFX/EXPEDITION/dash_3.wav");

            GPC_ASSETS->AddAudio("MiningStone1", "Resources/Audio/SFX/EXPEDITION/mining_stone_1.wav");
            GPC_ASSETS->AddAudio("MiningStone2", "Resources/Audio/SFX/EXPEDITION/mining_stone_2.wav");
            GPC_ASSETS->AddAudio("MiningStone3", "Resources/Audio/SFX/EXPEDITION/mining_stone_3.wav");
            GPC_ASSETS->AddAudio("MiningWood1", "Resources/Audio/SFX/EXPEDITION/mining_wood_1.wav");
            GPC_ASSETS->AddAudio("MiningWood2", "Resources/Audio/SFX/EXPEDITION/mining_wood_2.wav");
            GPC_ASSETS->AddAudio("MiningWood3", "Resources/Audio/SFX/EXPEDITION/mining_wood_3.wav");

            GPC_ASSETS->AddAudio("Attack1", "Resources/Audio/SFX/metal_glove_hit_1.wav");
            GPC_ASSETS->AddAudio("Attack2", "Resources/Audio/SFX/EXPEDITION/metal_glove_hit_2.wav");
            GPC_ASSETS->AddAudio("Attack3", "Resources/Audio/SFX/EXPEDITION/metal_glove_hit_3.wav");

            // EXPEDITION MONSTER
            GPC_ASSETS->AddAudio("EnemiCry1", "Resources/Audio/SFX/ENNEMIE/monster_cry_1.wav");
            GPC_ASSETS->AddAudio("EnemiCry2", "Resources/Audio/SFX/ENNEMIE/monster_cry_2.wav");
            GPC_ASSETS->AddAudio("EnemiCry3", "Resources/Audio/SFX/ENNEMIE/monster_cry_3.wav");
            GPC_ASSETS->AddAudio("EnemiCry4", "Resources/Audio/SFX/ENNEMIE/monster_cry_4.wav");
            GPC_ASSETS->AddAudio("EnemiFoot1", "Resources/Audio/SFX/ENNEMIE/monster_footstep_1.wav");
            GPC_ASSETS->AddAudio("EnemiFoot2", "Resources/Audio/SFX/ENNEMIE/monster_footstep_2.wav");
            GPC_ASSETS->AddAudio("EnemiFoot3", "Resources/Audio/SFX/ENNEMIE/monster_footstep_3.wav");
            GPC_ASSETS->AddAudio("EnemiFoot4", "Resources/Audio/SFX/ENNEMIE/monster_footstep_4.wav");

            // UI
            GPC_ASSETS->AddAudio("Clic", "Resources/Audio/SFX/Clic.wav");
            GPC_ASSETS->AddAudio("UI1", "Resources/Audio/SFX/sfx_ui1.wav");
            GPC_ASSETS->AddAudio("UI2", "Resources/Audio/SFX/sfx_ui2.wav");
            GPC_ASSETS->AddAudio("UI3", "Resources/Audio/SFX/sfx_ui3.wav");
            GPC_ASSETS->AddAudio("UI4", "Resources/Audio/SFX/sfx_ui4.wav");
            GPC_ASSETS->AddAudio("Hover", "Resources/Audio/SFX/sfx_hoover.wav");
            GPC_ASSETS->AddAudio("Ecran_titre", "Resources/Audio/Music/Ruins_of_Hestia_ecran_titre.wav");

            // VILLAGE
            GPC_ASSETS->AddAudio("Poser_batiment", "Resources/Audio/SFX/Poser batiment.wav");
            GPC_ASSETS->AddAudio("Alarme1", "Resources/Audio/SFX/alarme 1.wav");
            GPC_ASSETS->AddAudio("Alarme2", "Resources/Audio/SFX/alarme 2.wav");
            GPC_ASSETS->AddAudio("Alarme3", "Resources/Audio/SFX/alarme 3.wav");
            GPC_ASSETS->AddAudio("Upgrade", "Resources/Audio/SFX/sfx_upgrade.wav");
            GPC_ASSETS->AddAudio("ROH_village_intro", "Resources/Audio/Music/Ruins_of_hestia_village_intro.wav");
            GPC_ASSETS->AddAudio("ROH_village", "Resources/Audio/Music/Ruins_of_hestia_village_main_loop.wav");
            GPC_ASSETS->AddAudio("Nature", "Resources/Audio/Music/nature.wav");
            GPC_ASSETS->AddAudio("Nature_only_wind", "Resources/Audio/Music/nature (only wind).wav");
            GPC_ASSETS->AddAudio("Nature_only_birds", "Resources/Audio/Music/nature (only birds).wav");
        }

        return ErrorType::SUCCESS;
    }

    void SceneDefault::OnAssetsDestroy()
    {

    }

    void SceneDefault::OnSceneUpdate()
    {
        Scene::OnSceneUpdate();

        if (!kDebugOverlayEnabled) return;

        m_FPSSampleCount++;
        m_Accumulator += m_Clock.GetDeltaTimeUnscaled();

        if (m_Accumulator > 1.0f)
        {
            m_FpsReactive.Set("FPS " + std::to_string((int)(1.0f / (m_Accumulator / m_FPSSampleCount))));
            m_Accumulator = 0.0f;
            m_FPSSampleCount = 0;
        }

        std::stringstream mouse;
        mouse << std::fixed << std::setprecision(1);
        mouse << "Mouse Coords : { " << Inputs::GetMouseX() << ", " << Inputs::GetMouseY() << " }";
        m_MouseReactive.Set(mouse.str());

        m_EntityReactive.Set("Entity Count : " + std::to_string(m_EntityManager.GetLivingEntitiesCount()));

        if (mp_MainCameraTransform)
        {
            std::stringstream fwd;
            fwd << std::fixed << std::setprecision(2);
            fwd << "Forward : x" << mp_MainCameraTransform->LocalTransform.GetForward().x;
            fwd << " y" << mp_MainCameraTransform->LocalTransform.GetForward().y;
            fwd << " z" << mp_MainCameraTransform->LocalTransform.GetForward().z;
            m_LookingReactive.Set(fwd.str());

            std::stringstream pos;
            pos << std::fixed << std::setprecision(2);
            pos << "Position : x" << mp_MainCameraTransform->LocalTransform.GetPosition().x;
            pos << " y" << mp_MainCameraTransform->LocalTransform.GetPosition().y;
            pos << " z" << mp_MainCameraTransform->LocalTransform.GetPosition().z;
            m_CameraReactive.Set(pos.str());
        }
    }

    ErrorType SceneDefault::OnCreate(SceneInformation &info) {

        auto camera             = CreateEntityAs3D();
        mp_MainCamera            = AddComponent<CameraComponent>(camera);
        mp_MainCameraTransform   = GetComponent<GPC::Transform3D>(camera);

        auto globalLight = CreateEntityAs3D();
        mp_DefaultLight = AddComponent<LightingComponent>(globalLight);
        auto* glTransform = GetComponent<GPC::Transform3D>(globalLight);
        mp_DefaultLight->pLightData = GPC::LightFactory::CreateAsDirectional().Diffuse(Colors::WHITE).Intensity(20.0f).Build();
        glTransform->LocalTransform.RotateYawPitchRoll(0.0f, GPC_PI/2.0f, 0.0f);

        return ErrorType::SUCCESS;
    }

    void SceneDefault::CreateDebugOverlay()
    {
        if (!kDebugOverlayEnabled) return;

        UiBuilder b(this);

        mp_DebugRoot = b.At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                       .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
                       .Tint(Colors::TRANSPARENT)
                       .FitToScreen(true)
                       .BuildCanvas();
        if (!mp_DebugRoot) return;

        auto makeLine = [&](glm::vec2 pos, Color tint, UiReactive<std::string>& src) -> UiText*
        {
            UiText* t = b.ChildOf(mp_DebugRoot->GetEntityID())
                          .At(Anchors::TOP_LEFT, pos)
                          .Font(UiTheme::DefaultFont, 16.0f)
                          .Tint(tint)
                          .Text("")
                          .BuildText();
            if (t) t->BindReactive(src);
            return t;
        };

        mp_FpsLabel     = makeLine({ 20.0f,  5.0f }, Colors::AMETHYST, m_FpsReactive);
        mp_MouseLabel   = makeLine({ 20.0f, 25.0f }, Colors::BLACK,    m_MouseReactive);
        mp_EntityLabel  = makeLine({ 20.0f, 45.0f }, Colors::WHITE,    m_EntityReactive);
        mp_LookingLabel = makeLine({ 20.0f, 65.0f }, Colors::WHITE,    m_LookingReactive);
        mp_CameraLabel  = makeLine({ 20.0f, 85.0f }, Colors::WHITE,    m_CameraReactive);
    }


} // GPC
