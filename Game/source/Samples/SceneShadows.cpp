#include "Samples/SceneShadows.h"

#include "Assets.h"
#include "CameraSystem.h"
#include "FbxLoader.h"
#include "FreeCamera.h"
#include "LightingSystem.h"
#include "../../include/Expedition/PlayerController.h"
#include "Render3DSystem.h"
#include "DebugBehavior/MoveDebug.h"

namespace GPC
{
    ErrorType SceneShadows::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();

        return ErrorType::SUCCESS;
    }

    ErrorType SceneShadows::OnCreate(SceneInformation &info) {

        SceneDefault::OnCreate(info);
        CreateDebugOverlay();
        //AddBehavior<GPC::FreeCamera>(mp_MainCamera->GetEntityID());

        auto plateform = CreateEntityAs3D();
        auto* pPlateformRender = AddComponent<GPC::Render3DComponent>(plateform);
        auto* pPlateformTransform = GetComponent<GPC::Transform3D>(plateform);
        pPlateformRender->pGeometry = GPC_MESH("Cube");
        pPlateformRender->RenderMaterial.Diffuse = GPC_TEXTURE_ID("1024x1024", "Parquet");
        pPlateformRender->UseShadow = true;
        pPlateformTransform->LocalTransform.SetPosition(0, 0, 0);
        pPlateformTransform->LocalTransform.SetScale(100.0f, 1.0f, 100.0f);

        constexpr int gridSizeX = 10;
        constexpr int gridSizeY = 10;
        constexpr float spacing = 15.0f;
        constexpr float scaling = 5.0f;
        glm::vec3 plateformPosition = pPlateformTransform->LocalTransform.GetPosition();

        float offsetX = (gridSizeX - 1) * spacing * 0.5f;
        float offsetY = (gridSizeY - 1) * spacing * 0.5f;

        for (int i = 0; i < gridSizeX; i++)
        {
            for (int j = 0; j < gridSizeY; j++)
            {
                const auto cube = CreateEntityAs3D();
                auto* pRender = AddComponent<GPC::Render3DComponent>(cube);
                auto* pTransform = AddComponent<GPC::Transform3D>(cube);

                pRender->pGeometry = GPC_MESH("Cube");
                pRender->RenderMaterial.Diffuse = GPC_TEXTURE_ID("1024x1024", "Carbon");
                pRender->UseShadow = true;

                float posX = plateformPosition.x + i * spacing - offsetX;
                float posZ = plateformPosition.z + j * spacing - offsetY;

                float posY = plateformPosition.y + 1.0f * scaling;

                pTransform->LocalTransform.SetPosition(posX, posY, posZ);
                pTransform->LocalTransform.SetScale(scaling, scaling, scaling);
            }
        }


        FbxLoader loader;
        loader.Load("FBX_1", "Resources/Models/trophe.fbx");
        loader.Load("Cali", "Resources/Models/Characters/cali.fbx");

        const auto lTransform = GetComponent<Transform3D>(mp_DefaultLight->GetEntityID());
        lTransform->LocalTransform.SetRotationYawPitchRoll( glm::radians(10.0f), glm::radians(60.0f), 0 );

        auto player1 = CreateEntityAs3D();
        auto* pRenderer1 = AddComponent<GPC::Render3DComponent>(player1);
        auto* pTransform1 = GetComponent<GPC::Transform3D>(player1);
        mp_PlayerControllerBehavior = AddBehavior<GPC::PlayerController>(player1);
        pRenderer1->pGeometry = GPC_MESH("Cali");
        pTransform1->LocalTransform.SetPosition(2, 0.8f, 2);
        pTransform1->LocalTransform.SetScale(0.01, 0.01, 0.01);
        pRenderer1->RenderMaterial.Diffuse = GPC_TEXTURE_ID("1024x1024", "Carbon");
        pRenderer1->UseShadow = true;




        //mp_MainCameraTransform->LocalTransform.SetPosition(glm::vec3{-3, 8, -3});

        constexpr float CAMERA_DISTANCE = 100.0f * GENERAL_SCALE_EXPEDITION;
        mp_MainCamera->UseOrthographic(1.5f * GENERAL_SCALE_EXPEDITION);

        mp_MainCameraTransform->LocalTransform.SetRotationYawPitchRoll({glm::radians(225.0f), glm::radians(-45.0f), 0});
        mp_MainCameraTransform->LocalTransform.SetPosition(pTransform1->GetWorldPosition() + mp_MainCameraTransform->LocalTransform.GetForward() * CAMERA_DISTANCE);

        mp_MainCameraBehavior = AddBehavior<FollowTarget>(mp_MainCamera->GetEntityID());
        mp_MainCameraBehavior->LerpSpeed = 8.0f;
        mp_MainCameraBehavior->pTarget = GetComponent<Transform3D>(mp_PlayerControllerBehavior->GetEntityID());
        mp_MainCameraBehavior->Offset = mp_MainCameraTransform->LocalTransform.GetForward() * CAMERA_DISTANCE + glm::vec3( 3.0f, 0.0f, 3.0f );

        return ErrorType::SUCCESS;
    }

    void SceneShadows::OnSceneUpdate()
    {
        SceneDefault::OnSceneUpdate();

        static float Angle = 0.0f;
        Angle += 0.00016 * 1.5f;

        //const auto lTransform = GetComponent<Transform3D>(mp_DefaultLight->GetEntityID());
        //lTransform->LocalTransform.SetRotationYawPitchRoll( glm::cos(Angle), glm::sin(Angle), 0.2f );


    }

    void SceneShadows::OnDestroy() {

    }
} // GPC