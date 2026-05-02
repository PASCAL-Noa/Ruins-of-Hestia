#include "Map Generation/EnnemiGeneratorBehavior.h"

#include "ArchiveManager.h"
#include "Render3DSystem.h"
#include "Scene.h"
#include "Expedition/EnnemiBehavior.h"
#include "Expedition/HealthBarController.h"
#include "Expedition/LivingEntityBehavior.h"
#include "Expedition/BossBarHandler.h"

namespace GPC {
    void EnnemiGeneratorBehavior::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);
        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
    }

    void EnnemiGeneratorBehavior::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);

        auto pGeneration = GPC::MapGeneratorFactory::GetGeneration(GenerationID);

        glm::ivec2 start = pGeneration->GeneratedLayout.GetStart();
        glm::ivec2 end = pGeneration->GeneratedLayout.GetEnd();

        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                if (x_room == start.x && y_room == start.y) continue;

                bool should_spawn_boss = x_room == end.x && y_room == end.y;

                auto* pMeta = pGeneration->GeneratedMissionData.GetMeta(x_room, y_room);

                if (pMeta == nullptr) continue;
                for (auto& spawner : pMeta->GetEnnemies()) {

                    glm::vec2 input = { ROOM_SIZE - spawner.position.x - 0.5f, spawner.position.y + 0.5f };
                    glm::vec2 centered_input = input - glm::vec2{ROOM_SIZE, ROOM_SIZE} * 0.5f;
                    glm::vec2 rotated_input = pGeneration->GeneratedMissionData.GetRotation(x_room, y_room) * centered_input;
                    glm::vec2 normalized_input = 2.0f * rotated_input / glm::vec2(ROOM_SIZE, ROOM_SIZE);

                    glm::vec2 position = {
                        MapGeneratorData::ROOM_SCALE * (static_cast<float>(x_room) + 0.5f) + ROOM_SIZE * normalized_input.x * MapGeneratorData::TILE_GROUP_SCALE * 0.5f,
                        MapGeneratorData::ROOM_SCALE * (static_cast<float>(y_room) + 0.5f) + ROOM_SIZE * normalized_input.y * MapGeneratorData::TILE_GROUP_SCALE * 0.5f
                    };


                    if (should_spawn_boss && mp_Boss == nullptr) {
                        CreateBoss(position, { x_room, y_room }, pCtx->pScene);
                    } else {
                        CreateEnnemi(position, { x_room, y_room }, pCtx->pScene);
                    }
                }

            }
        }

        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                auto& ennemies = m_SpawnedByRoom[x_room][y_room];
                for (uint32_t i = 0; i < ennemies.Array.size() && i < ennemies.Count; ++i) {
                    ennemies.Array[i].pBehavior->pComponents = ennemies.Array.data() + i;
                    ennemies.Array[i].pBehavior->pManager = this;
                    ennemies.Array[i].pBehavior->pPlayer = pPlayerTransform;
                }
            }
        }

        m_CurrentRoom = GPC::WorldPositionToRoomPosition(pPlayerTransform->LocalTransform.GetPosition());
        UpdateVisibilityBasedOnDistance();
    }

    void EnnemiGeneratorBehavior::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (pPlayerTransform == nullptr) return;


        m_EnemiWalkTimer -= pCtx->pClock->GetDeltaTime();
        if (m_EnemiWalkTimer <= 0.0f)
        {
            if (pEnemiWalk) pEnemiWalk->Play();
            m_EnemiWalkTimer = 0.35f;
        }
        glm::ivec2 currentRoom = GPC::WorldPositionToRoomPosition(pPlayerTransform->LocalTransform.GetPosition());
        if (m_CurrentRoom.x != currentRoom.x || m_CurrentRoom.y != currentRoom.y) {
            m_CurrentRoom = currentRoom;
            UpdateVisibilityBasedOnDistance();
        }
    }

    EnnemiGeneratorBehavior::EnnemiComponents * EnnemiGeneratorBehavior::GetClosestEnnemi(float &out_distance, glm::vec2 &out_direction) {

        glm::ivec2 room_position = WorldPositionToRoomPosition(pPlayerTransform->LocalTransform.GetPosition());

        float min_sq_distance = FLT_MAX;
        glm::vec2 min_diff = {0, 0};
        EnnemiComponents* pComponents = nullptr;

        for (int32_t dx = -1; dx <= 1; dx++) {
            for (int32_t dy = -1; dy <= 1; dy++) {
                int32_t x = room_position.x + dx;
                int32_t y = room_position.y + dy;
                if (x < 0 || y < 0 || x > MAX_MAP_SIZE - 1 || y > MAX_MAP_SIZE - 1) continue;

                auto& ennemis = m_SpawnedByRoom[x][y];

                for (uint32_t i = 0; i < ennemis.Array.size() && i < ennemis.Count; ++i) {
                    if (ennemis.Array[i].IsAlive == false) continue;

                    glm::vec2 diff = {
                        ennemis.Array[i].pTransform->GetWorldPosition().x - pPlayerTransform->LocalTransform.GetPosition().x,
                        ennemis.Array[i].pTransform->GetWorldPosition().z - pPlayerTransform->LocalTransform.GetPosition().z
                    };
                    float sq_dist = glm::dot(diff, diff);
                    if (sq_dist < min_sq_distance) {
                        min_sq_distance = sq_dist;
                        pComponents = ennemis.Array.data() + i;
                        min_diff = diff;
                    }
                }
            }
        }


        out_distance = glm::sqrt(min_sq_distance);
        out_direction = min_diff / out_distance;
        return pComponents;
    }

    EnnemiGeneratorBehavior::EnnemiComponents * EnnemiGeneratorBehavior::GetBoss() {
        return mp_Boss;
    }

    void EnnemiGeneratorBehavior::KillEnnemi(EnnemiComponents *pEnnemi, Scene* pScene) {
        KillEnnemiByEID(pEnnemi->EID, pEnnemi->Room, pScene);
    }

    void EnnemiGeneratorBehavior::KillEnnemiByEID(EntityID eid, glm::ivec2 room, Scene *pScene) {

        auto& ennemis = m_SpawnedByRoom[room.x][room.y];
        for (uint32_t iEnnemi = 0; iEnnemi < ennemis.Array.size() && iEnnemi < ennemis.Count; ++iEnnemi) {
            if (ennemis.Array[iEnnemi].EID == eid && ennemis.Array[iEnnemi].IsAlive) {
                auto entity = ennemis.Array[iEnnemi].EID;
                auto health_bar = ennemis.Array[iEnnemi].HealthBar;
                // ennemis.Array[iEnnemi] = ennemis.Array[ennemis.Count - 1];
                ennemis.Array[iEnnemi].IsAlive = false;
                ennemis.Array[iEnnemi].pLiving = nullptr;
                pScene->DestroyEntity(entity);
                pScene->DestroyEntity(health_bar);
                return;
            }
        }
    }

    void EnnemiGeneratorBehavior::DestroyAllEnnemies(Scene *pScene) {
        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                auto& ennemies = m_SpawnedByRoom[x_room][y_room];
                for (uint32_t i = 0; i < ennemies.Array.size() && i < ennemies.Count; ++i) {
                    if (ennemies.Array[i].IsAlive == true)
                    {
                        ennemies.Array[i].pLiving = nullptr;
                        ennemies.Array[i].IsAlive = false;
                        pScene->DestroyEntity(ennemies.Array[i].EID);
                        pScene->DestroyEntity(ennemies.Array[i].HealthBar);
                    }
                }
            }
        }
    }

    void EnnemiGeneratorBehavior::UpdateVisibilityBasedOnDistance() {
        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                glm::ivec2 abs_diff = glm::abs(m_CurrentRoom - glm::ivec2{ x_room, y_room });
                const bool IS_NEIGHBOR = abs_diff.x <= 1 && abs_diff.y <= 1;
                auto& ennemies = m_SpawnedByRoom[x_room][y_room];
                for (uint32_t i = 0; i < ennemies.Array.size() && i < ennemies.Count; ++i) {
                    if (ennemies.Array[i].IsAlive == false) continue;
                    ennemies.Array[i].pRenderer->IsEnable = IS_NEIGHBOR;
                    if (ennemies.Array[i].pHealthBarRenderer != nullptr)
                        ennemies.Array[i].pHealthBarRenderer->IsEnable = IS_NEIGHBOR;
                    // ennemies.Array[i].pAnimation->IsEnable = IS_NEIGHBOR;
                    ennemies.Array[i].pBehavior->SetEnabled(IS_NEIGHBOR);
                }
            }
        }
    }

    void EnnemiGeneratorBehavior::CreateEnnemi(glm::vec2 position, glm::ivec2 room, Scene *pScene) {
        if (m_SpawnedByRoom[room.x][room.y].Count >= MAX_ENNEMI_BY_ROOM) return;

        auto ennemi = pScene->CreateEntityAs3D();

        auto eTransform = pScene->GetComponent<Transform3D>(ennemi);
        eTransform->SetParent(mp_Transform);
        eTransform->LocalTransform.SetScale(RESCALE_ENNEMI * GENERAL_SCALE_EXPEDITION,RESCALE_ENNEMI * GENERAL_SCALE_EXPEDITION, RESCALE_ENNEMI * GENERAL_SCALE_EXPEDITION);
        eTransform->LocalTransform.SetPosition(position.x, 0.3f * GENERAL_SCALE_EXPEDITION * 0.5f, position.y);

        auto eRenderer = pScene->AddComponent<Render3DComponent>(ennemi);
        eRenderer->pGeometry = GPC_MESH("Ennemi");
        eRenderer->SetTexture("2048x2048", "EnnemiTexture");

        auto eCollider = pScene->AddComponent<Collision3DComponent>(ennemi);
        eCollider->IsSphere = true;
        eCollider->CollType = ColliderType::Enemy;

        // Performance issues :
        // auto eAnimation = pScene->AddComponent<AnimationComponent>(ennemi);
        // eAnimation->SetFbxData(pMeshData);
        // eAnimation->Looping = true;
        // eAnimation->Playing = true;
        // eAnimation->ChangeAnimationTo("Idle");

        auto eBehavior = pScene->AddBehavior<EnnemiBehavior>(ennemi);

        auto eLiving = pScene->AddBehavior<LivingEntityBehavior>(ennemi);
        eLiving->InitAsEnnemi(Difficulty);

        auto HealthBar = pScene->CreateEntityAs3D();

        auto hbTransform = pScene->GetComponent<Transform3D>(HealthBar);
        hbTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI);

        auto hbRenderer = pScene->AddComponent<Render3DComponent>(HealthBar);
        hbRenderer->SetTexture("1024x1024", "Red");
        hbRenderer->pGeometry = GPC_MESH("Plane");

        auto hbController = pScene->AddBehavior<HealthBarController>(HealthBar);

        m_SpawnedByRoom[room.x][room.y].Array[m_SpawnedByRoom[room.x][room.y].Count] = {
            true,
            room,
            ennemi,
            HealthBar,
            eTransform,
            eRenderer,
            eCollider,
            eBehavior,
            eLiving,
            hbTransform,
            hbRenderer,
            // eAnimation
        };

        hbController->pComponents = &m_SpawnedByRoom[room.x][room.y].Array[m_SpawnedByRoom[room.x][room.y].Count];
        eLiving->pData = &m_SpawnedByRoom[room.x][room.y].Array[m_SpawnedByRoom[room.x][room.y].Count];
        eBehavior->mp_Living = eLiving;
        m_SpawnedByRoom[room.x][room.y].Count++;
    }

    void EnnemiGeneratorBehavior::CreateBoss(glm::vec2 position, glm::ivec2 room, Scene *pScene) {
        if (m_SpawnedByRoom[room.x][room.y].Count >= MAX_ENNEMI_BY_ROOM) return;

        auto ennemi = pScene->CreateEntityAs3D();

        auto eTransform = pScene->GetComponent<Transform3D>(ennemi);
        eTransform->SetParent(mp_Transform);
        eTransform->LocalTransform.SetScale(RESCALE_BOSS * GENERAL_SCALE_EXPEDITION,RESCALE_BOSS * GENERAL_SCALE_EXPEDITION, RESCALE_BOSS * GENERAL_SCALE_EXPEDITION);
        eTransform->LocalTransform.SetPosition(position.x, 0.3f * GENERAL_SCALE_EXPEDITION * 0.5f, position.y);

        auto eRenderer = pScene->AddComponent<Render3DComponent>(ennemi);
        eRenderer->pGeometry = GPC_MESH("Boss");
        eRenderer->SetTexture("2048x2048", "BossTexture");

        auto eCollider = pScene->AddComponent<Collision3DComponent>(ennemi);
        eCollider->IsSphere = true;
        eCollider->CollType = ColliderType::Enemy;

        // Performance issues :
        // auto eAnimation = pScene->AddComponent<AnimationComponent>(ennemi);
        // eAnimation->SetFbxData(pMeshData);
        // eAnimation->Looping = true;
        // eAnimation->Playing = true;
        // eAnimation->ChangeAnimationTo("Idle");

        auto eBehavior = pScene->AddBehavior<EnnemiBehavior>(ennemi);

        auto eLiving = pScene->AddBehavior<LivingEntityBehavior>(ennemi);
        eLiving->InitAsBoss(Difficulty);


        auto pGeneration = GPC::MapGeneratorFactory::GetGeneration(GenerationID);

        auto eHealthBar = pScene->AddBehavior<BossBarHandler>(ennemi);
        eHealthBar->pMainCanvas = pMainCanvas;
        eHealthBar->EndRoom = pGeneration->GeneratedLayout.GetEnd();

        m_SpawnedByRoom[room.x][room.y].Array[m_SpawnedByRoom[room.x][room.y].Count] = {
            true,
            room,
            ennemi,
            MAX_ENTITIES,
            eTransform,
            eRenderer,
            eCollider,
            eBehavior,
            eLiving,
            nullptr,
            nullptr
            // eAnimation
        };

        mp_Boss = &m_SpawnedByRoom[room.x][room.y].Array[m_SpawnedByRoom[room.x][room.y].Count];
        eHealthBar->pBoss = mp_Boss;

        eLiving->pData = &m_SpawnedByRoom[room.x][room.y].Array[m_SpawnedByRoom[room.x][room.y].Count];
        eBehavior->mp_Living = eLiving;
        m_SpawnedByRoom[room.x][room.y].Count++;

    }
} // GPC