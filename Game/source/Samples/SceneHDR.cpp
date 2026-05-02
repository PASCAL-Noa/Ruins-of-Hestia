#include "Samples/SceneHDR.h"

#include "FbxLoader.h"
#include "FreeCamera.h"
#include "LightingSystem.h"
#include "Render3DSystem.h"

namespace GPC
{
    ErrorType SceneHDR::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();

        {
            GPC_ASSETS->AddTexture("WALL_DIFFUSE",  "Resources/Materials/BRICK/dwall.jpg");
            GPC_ASSETS->AddTexture("WALL_NORMAL",   "Resources/Materials/BRICK/nwall.jpg");
            // GPC_ASSETS->AddTexture("WALL_HEIGHT",   "Resources/Materials/BRICK/hwall.png");

            GPC_ASSETS->AddTextureArray(
                "1024x1024",
                1024, 1024,
                {
                    GPC_TEXTURE("WALL_DIFFUSE"),
                    GPC_TEXTURE("WALL_NORMAL"),
                    // GPC_TEXTURE("WALL_HEIGHT"),
                }
            );

            GPC_GRAPHIC_PROGRAM("Default Program")->Program.InitializeWith({&GPC_TEXTURE_ARRAY("HDR_TEXTURE")->Array}, GPC_TEXTURE_ARRAY("HDR_TEXTURE")->TextureSampler);
        }

        return ErrorType::SUCCESS;

    }

    void SceneHDR::OnDestroy()
    {
    }

    ErrorType SceneHDR::OnCreate(SceneInformation &info)
    {
        SceneDefault::OnCreate(info);

        // CreateDebugOverlay();

        AddBehavior<FreeCamera>(mp_MainCamera->GetEntityID());

        auto* rComp = GetComponent<Render3DComponent>(CreateSphere({0, 0, 5},     {2, 2, 2}));
        rComp->SetTexture("1024x1024", "WALL_DIFFUSE", "WALL_NORMAL");
        rComp = GetComponent<Render3DComponent>(CreateSphere({0, 2, 0},     {2, 2, 2}));
        rComp->SetTexture("1024x1024", "WALL_DIFFUSE", "WALL_NORMAL");
        rComp = GetComponent<Render3DComponent>(CreateSphere({0, -2, 0},    {2, 2, 2}));
        rComp->SetTexture("1024x1024", "WALL_DIFFUSE", "WALL_NORMAL");
        rComp = GetComponent<Render3DComponent>(CreateSphere({-2, 0, 0},    {2, 2, 2}));
        rComp->SetTexture("1024x1024", "WALL_DIFFUSE", "WALL_NORMAL");
        CreateSphere({2, 0, 0},     {2, 2, 2});

        auto e = CreateSphere({0, 0, -5}, {1, 1, 1});
        rComp = GetComponent<Render3DComponent>(e);
        rComp->pGeometry = GPC_MESH("Cube");
        rComp->SetTexture("1024x1024", "WALL_DIFFUSE", "WALL_NORMAL");

        auto wall = CreatePlane({2, 0, 3}, {1, 1, 1});
        rComp = GetComponent<Render3DComponent>(wall);
        rComp->SetTexture("1024x1024", "WALL_DIFFUSE", "WALL_NORMAL");
        auto wTran = GetComponent<Transform3D>(wall);
        wTran->LocalTransform.RotateYawPitchRoll(0, GPC_PI/2.0f, GPC_PI/2.0f);

        auto wall1 = CreatePlane({1, 0, 3}, {1, 1, 1});
        rComp = GetComponent<Render3DComponent>(wall1);
        rComp->SetTexture("1024x1024", "WALL_DIFFUSE");
        auto wTran1 = GetComponent<Transform3D>(wall1);
        wTran1->LocalTransform.RotateYawPitchRoll(0, GPC_PI/2.0f, GPC_PI/2.0f);


        // LIGHTS
        auto light = CreateSphere({0, 0, 3}, "HDR_TEXTURE");
        auto* rLight  = GetComponent<Render3DComponent>(light);
        rLight->RenderMaterial.EmissiveStrength = 1.0;
        rLight->RenderMaterial.Tint = Colors::WHITE;
        auto* lComp     = AddComponent<LightingComponent>(light);
        lComp->pLightData = GPC_POINT_LIGHT("PointWhite");

        return ErrorType::SUCCESS;
    }
} // GPC