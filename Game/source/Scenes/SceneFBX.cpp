#include "Scenes/SceneFBX.h"
#include "AnimationComponent.h"
#include "FbxLoader.h"
#include "FreeCamera.h"
#include "Inputs.h"
#include "LightingSystem.h"
#include "Render3DSystem.h"

namespace GPC
{
    ErrorType SceneFBX::OnAssetsLoad()
    {
        mp_FbxData = GPC_FBX->Load("Cali", "Resources/Models/Characters/Cali_FullAnimation.fbx");
        if (!mp_FbxData || !mp_FbxData->pGeometryData)
            return ErrorType::FILE_READ_FAILED;

        return SceneDefault::OnAssetsLoad();
    }

    void SceneFBX::OnSceneUpdate()
    {
        // Change stack index with an input for testing
        auto* animation = GetComponent<AnimationComponent>(m_AnimationEntity);
        if (!animation || !mp_FbxData || !mp_FbxData->pAnimationData) { return; }

        if (Inputs::IsKeyPress(Inputs::KeyCode::V))
        {
            animation->ChangeAnimationTo("Idle");
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::B))
        {
            animation->ChangeAnimationTo("Running");
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::N))
        {
            animation->ChangeAnimationTo("Punching");
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::G))
        {
            animation->ChangeAnimationTo(2);
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::H))
        {
            animation->ChangeAnimationTo(3);
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::J))
        {
            animation->ChangeAnimationTo(0);
        }

        SceneDefault::OnSceneUpdate();
    }

    ErrorType SceneFBX::OnCreate(SceneInformation &info)
    {
        SceneDefault::OnCreate(info);

        auto cController = AddBehavior<FreeCamera>(mp_MainCamera->GetEntityID());

        cController->MovementSpeed = 100;

        if (mp_FbxData)
        {
            m_AnimationEntity = CreateEmpty();
            auto animation = AddComponent<AnimationComponent>(m_AnimationEntity);
            animation->SetFbxData(mp_FbxData);
            animation->Looping = true;
            animation->Speed = 1.0f;
            animation->Time = 0.0;
            animation->EnableCpuSkinning = true;
            animation->StackIndex = 0;
        }

        FbxMeshData* currentFbxData = mp_FbxData;
        while (currentFbxData)
        {
            auto obj = CreateEntityAs3D();
            auto pRenderer = AddComponent<Render3DComponent>(obj);

            pRenderer->pGeometry = GPC_MESH(currentFbxData->MeshName);
            pRenderer->SetTexture("1024x1024", "Grey");
            auto oTransform = GetComponent<Transform3D>(obj);
            oTransform->LocalTransform.SetPosition(currentFbxData->Position);
            oTransform->LocalTransform.SetRotation(currentFbxData->Rotation);
            oTransform->LocalTransform.SetScale(currentFbxData->Scale);

            currentFbxData = currentFbxData->pNext;
        }

        mp_MainCameraTransform->LocalTransform.AddPosition(1, 1, 1);

        auto light0 = CreateEntityAs3D();
        auto lLight0 = AddComponent<LightingComponent>(light0);
        auto lTransform0 = GetComponent<Transform3D>(light0);
        lLight0->pLightData = GPC_POINT_LIGHT("PointWhite");
        lTransform0->LocalTransform.AddPosition(2, 3, 2);
        return ErrorType::SUCCESS;
    }

    void SceneFBX::OnDestroy()
    {
    }
}
