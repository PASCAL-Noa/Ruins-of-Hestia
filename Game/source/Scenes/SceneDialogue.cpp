#include "Scenes/SceneDialogue.h"

#include "FreeCamera.h"
#include "Dialogue/DialogueBehaviour.h"
#include "UI/UiCanvas.h"

namespace GPC
{
    ErrorType SceneDialogue::OnAssetsLoad()
    {
        return SceneDefault::OnAssetsLoad();
    }

    ErrorType SceneDialogue::OnCreate(SceneInformation& info)
    {
        SceneDefault::OnCreate(info);

        CreateDebugOverlay();

        CreateFreeCamera();
        SetupUI();

        CreateSunLighting();
        CreateGrounds();



        return ErrorType::SUCCESS;

    }

    void SceneDialogue::OnSceneUpdate()
    {
        SceneDefault::OnSceneUpdate();
    }

    void SceneDialogue::OnDestroy()
    {
    }

    void SceneDialogue::CreateFreeCamera()
    {
        auto camera_libre = CreateEntityAs3D();

        auto clTransform    = GetComponent<Transform3D>(camera_libre);
        clTransform->LocalTransform.SetPosition(0.0f, 1.0f, 0.0f);
        clTransform->LocalTransform.SetRotationYawPitchRoll(0, glm::radians(-45.0f), 0);

        auto mp_FreeCameraCamera = AddComponent<CameraComponent>(camera_libre);

        mp_FreeCameraBehavior = AddBehavior<FreeCamera>(camera_libre);
        mp_FreeCameraBehavior->MovementSpeed = 30.0f * GENERAL_SCALE_EXPEDITION;

        mp_MainCamera->Active = false;
        mp_FreeCameraCamera->Active = true;
        mp_FreeCameraBehavior->SetEnabled(true);
    }

    void SceneDialogue::CreateSunLighting()
    {
        auto lTransform = GetComponent<Transform3D>(mp_DefaultLight->GetEntityID());
        lTransform->LocalTransform.SetRotationYawPitchRoll( glm::radians(10.0f), glm::radians(60.0f), 0 );
    }

    void SceneDialogue::CreateGrounds()
    {
        auto plateform = CreateEntityAs3D();
        auto pRender = AddComponent<Render3DComponent>(plateform);
        auto pTransform = AddComponent<Transform3D>(plateform);
        pRender->pGeometry = GPC_MESH("Cube");
        pRender->SetTexture("1024x1024", "Blue");
        pTransform->LocalTransform.SetPosition(0, 0, 0);
        pTransform->LocalTransform.SetScale(1000, 1, 1000);
    }

    void SceneDialogue::SetupUI()
    {
        auto dialogue = CreateEntityAs2D();
        mp_DialogueBehaviour = AddBehavior<DialogueBehaviour>(dialogue);

    }
} // GPC