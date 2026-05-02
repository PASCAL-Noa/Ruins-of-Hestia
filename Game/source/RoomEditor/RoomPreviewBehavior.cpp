#include "RoomEditor/RoomPreviewBehavior.h"

#include "FreeCamera.h"
#include "Scene.h"
#include "RoomEditor/RoomEditorBehavior.h"

namespace GPC {

    void RoomPreviewBehavior::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);

        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());

        CreateTerrain(pCtx->pScene);
        CreateCamera(pCtx->pScene);
    }

    void RoomPreviewBehavior::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (Inputs::IsKeyPress(Inputs::KeyCode::E)) {
            pEditor->GoToEditMode();
        }
    }

    void RoomPreviewBehavior::GenerateWith(RoomMeta *pMeta, Scene *pScene) {
        ClearEditor(pScene);

        mp_MetaRoom = pMeta;

        GenerateTerrain(pScene);
        GenerateEnnemiSpawners(pScene);
        GenerateResourceSpawners(pScene);
        GeneratePlayerSpawner(pScene);

        mp_CameraComponent->Active = true;
        pEditor->SetEnabled(false);
        SetEnabled(true);
    }

    void RoomPreviewBehavior::GenerateTerrain(Scene *pScene) {
        auto* generation = GPC::MapGeneratorFactory::GetGeneration(m_Generation);
        generation->Generate1Room(mp_MetaRoom->GetRaw());
        mp_TerrainRenderer->pGeometry = generation->GeneratedRoomDatas[generation->GeneratedRoomDatas.size() - 1].pGeometry;
    }

    void RoomPreviewBehavior::GenerateEnnemiSpawners(Scene *pScene) {
        for (auto spawner : mp_MetaRoom->GetEnnemies()) {
            m_LivingSpawner.push_back(RoomEditorBehavior::CreateEnnemiSpawner(
                spawner.position,
                mp_Transform,
                pScene
            ));
        }
    }

    void RoomPreviewBehavior::GenerateResourceSpawners(Scene *pScene) {
        for (auto spawner : mp_MetaRoom->GetResources()) {
            m_LivingSpawner.push_back(RoomEditorBehavior::CreateResourceSpawner(
                spawner.position,
                mp_Transform,
                pScene
            ));
        }
    }

    void RoomPreviewBehavior::GeneratePlayerSpawner(Scene *pScene) {
        auto& player_spawn = mp_MetaRoom->GetPlayer();
        m_LivingSpawner.push_back(RoomEditorBehavior::CreatePlayerSpawner(
                player_spawn.position,
                mp_Transform,
                pScene
        ));
    }

    void RoomPreviewBehavior::CreateTerrain(Scene *pScene) {
        m_Generation = GPC::MapGeneratorFactory::CreateNew();

        auto terrain = pScene->CreateEntityAs3D();

        auto tTransform = pScene->GetComponent<Transform3D>(terrain);
        tTransform->SetParent(mp_Transform);
        tTransform->LocalTransform.SetPosition(-1.0f, 0.0f, -1.0f);

        mp_TerrainRenderer = pScene->AddComponent<Render3DComponent>(terrain);
        mp_TerrainRenderer->pGeometry = nullptr;
        mp_TerrainRenderer->SetTexture("1024x1024", "Seamless1");
    }

    void RoomPreviewBehavior::CreateCamera(Scene *pScene) {
        auto camera = pScene->CreateEntityAs3D();

        mp_CameraTransform = pScene->GetComponent<Transform3D>(camera);
        mp_CameraTransform->SetParent(mp_Transform);
        mp_CameraTransform->LocalTransform.SetPosition(ROOM_SIZE * 0.5f, 3.0f, ROOM_SIZE * 1.5f);

        mp_CameraComponent = pScene->AddComponent<CameraComponent>(camera);
        mp_CameraComponent->Active = false;

        auto freeCamera = pScene->AddBehavior<FreeCamera>(camera);
        freeCamera->MaxAnglePitch = GPC_PI * 0.45f;
        freeCamera->MinAnglePitch = -GPC_PI * 0.45f;
    }

    void RoomPreviewBehavior::ClearEditor(Scene* pScene) {
        mp_TerrainRenderer->pGeometry = nullptr;
        for (auto spawner : m_LivingSpawner) {
            pScene->DestroyEntity(spawner);
        }
        m_LivingSpawner.clear();
    }

}
