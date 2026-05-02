//
// Created by killi on 25/03/2026.
//

#include "Scenes/SceneMeta.h"
#include "Meta/MetaTypes.h"
#include "Tools/MetaGenerator.h"
#include <filesystem>

#include "CameraSystem.h"
#include "FreeCamera.h"
#include "LightingSystem.h"
#include "Render3DSystem.h"

namespace GPC
{
    ErrorType SceneMeta::OnAssetsLoad()
    {
        if (!std::filesystem::exists("Resources/Packs/scene_meta.meta"))
            Tools::GenerateSceneMetaPack();

        MetaError metaErr = GPC_ASSETS->LoadMeta("Resources/Packs/scene_meta.meta");
        if (metaErr != MetaError::SUCCESS)
            return ErrorType::FILE_READ_FAILED;

        MetaRuntimeContext ctx { .Window = mp_Window };
        metaErr = GPC_ASSETS->ResolveRuntimeAssets(ctx);
        if (metaErr != MetaError::SUCCESS)
            return ErrorType::FILE_READ_FAILED;

        GPC_GRAPHIC_PROGRAM("Default Program")->Program.InitializeWith(
            { &GPC_TEXTURE_ARRAY("MetaTextures")->Array },
            GPC_TEXTURE_ARRAY("MetaTextures")->TextureSampler
        );

        return ErrorType::SUCCESS;
    }

    void SceneMeta::OnAssetsDestroy()
    {
    }

    ErrorType SceneMeta::OnCreate(SceneInformation& info)
    {
        // Camera
        auto camera = CreateEntityAs3D();
        auto* cTransform = GetComponent<Transform3D>(camera);
        cTransform->LocalTransform.AddPosition(0, 2, 5);
        AddComponent<CameraComponent>(camera);
        AddBehavior<FreeCamera>(camera);

        // Cube
        auto cube = CreateEntityAs3D();
        auto* pTransform = GetComponent<Transform3D>(cube);
        pTransform->LocalTransform.AddPosition(0, 0, 0);
        auto* pRenderer = AddComponent<Render3DComponent>(cube);
        pRenderer->pGeometry = GPC_MESH("MetaCube");
        pRenderer->RenderMaterial.Diffuse = GPC_TEXTURE_ID("MetaTextures", "MetaTex");

        // Light
        auto light = CreateEntityAs3D();
        auto* lTransform = GetComponent<Transform3D>(light);
        lTransform->LocalTransform.AddPosition(2, 3, 2);
        auto* lLight = AddComponent<LightingComponent>(light);
        lLight->pLightData = GPC_POINT_LIGHT("MetaLight");

        return ErrorType::SUCCESS;
    }

    void SceneMeta::OnDestroy()
    {
    }
} // GPC
