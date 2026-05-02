#include "Map Generation/ResourceGeneratorBehavior.h"

#include "Random.h"
#include "Render3DSystem.h"
#include "Scene.h"
#include "Expedition/PlayerInteraction.h"
#include "Expedition/ResourceBehavior.h"
#include "UI/UiBuilder.h"

namespace GPC {
    void ResourceGeneratorBehavior::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);

        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
    }

    void ResourceGeneratorBehavior::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);

        auto pGeneration = GPC::MapGeneratorFactory::GetGeneration(GenerationID);

        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                auto* pMeta = pGeneration->GeneratedMissionData.GetMeta(x_room, y_room);
                if (pMeta == nullptr) continue;
                uint32_t count = 0;
                for (auto& spawner : pMeta->GetResources()) {

                    glm::vec2 input = { ROOM_SIZE - spawner.position.x - 0.5f, spawner.position.y + 0.5f };
                    glm::vec2 centered_input = input - glm::vec2{ROOM_SIZE, ROOM_SIZE} * 0.5f;
                    glm::vec2 rotated_input = pGeneration->GeneratedMissionData.GetRotation(x_room, y_room) * centered_input;
                    glm::vec2 normalized_input = 2.0f * rotated_input / glm::vec2(ROOM_SIZE, ROOM_SIZE);

                    glm::vec2 position = {
                        MapGeneratorData::ROOM_SCALE * (static_cast<float>(x_room) + 0.5f) + ROOM_SIZE * normalized_input.x * MapGeneratorData::TILE_GROUP_SCALE * 0.5f,
                        MapGeneratorData::ROOM_SCALE * (static_cast<float>(y_room) + 0.5f) + ROOM_SIZE * normalized_input.y * MapGeneratorData::TILE_GROUP_SCALE * 0.5f
                    };

                    CreateResource(position, { x_room, y_room }, pCtx->pScene);
                    count++;
                    if (count >= 50) break;
                }
            }
        }

        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {

                auto& resources = m_SpawnedByRoom[x_room][y_room];

                for (uint32_t iResource = 0; iResource < resources.Count; ++iResource) {
                    resources.Array[iResource].pBehavior->pComponents = &resources.Array[iResource];
                    resources.Array[iResource].pBehavior->pManager = this;
                }
            }
        }

        m_CurrentRoom = WorldPositionToRoomPosition(pPlayerTransform->LocalTransform.GetPosition());
        UpdateVisibilityBasedOnDistance();
    }

    void ResourceGeneratorBehavior::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (pPlayerTransform == nullptr) return;

        glm::ivec2 currentRoom = WorldPositionToRoomPosition(pPlayerTransform->GetWorldPosition());
        if (m_CurrentRoom.x != currentRoom.x || m_CurrentRoom.y != currentRoom.y) {
            m_CurrentRoom = currentRoom;
            UpdateVisibilityBasedOnDistance();
        };
    }

    ResourceGeneratorBehavior::ResourceComponents * ResourceGeneratorBehavior::GetClosestResource(float &out_distance, glm::vec2& out_direction) {
        glm::ivec2 room_position = WorldPositionToRoomPosition(pPlayerTransform->LocalTransform.GetPosition());

        float min_sq_distance = FLT_MAX;
        glm::vec2 min_diff{0, 0};
        ResourceComponents* pComponents = nullptr;
        for (int32_t dx = -1; dx <= 1; dx++) {
            for (int32_t dy = -1; dy <= 1; dy++) {
                int32_t x = room_position.x + dx;
                int32_t y = room_position.y + dy;
                if (x < 0 || y < 0 || x > MAX_MAP_SIZE - 1 || y > MAX_MAP_SIZE - 1) continue;

                auto& resources = m_SpawnedByRoom[room_position.x][room_position.y];

                for (uint32_t iResource = 0; iResource < resources.Count; ++iResource)
                {
                    glm::vec2 diff = {
                        resources.Array[iResource].pTransform1->GetWorldPosition().x - pPlayerTransform->LocalTransform.GetPosition().x,
                        resources.Array[iResource].pTransform1->GetWorldPosition().z - pPlayerTransform->LocalTransform.GetPosition().z
                    };
                    float sq_dist = glm::dot(diff, diff);
                    if (sq_dist < min_sq_distance) {
                        min_sq_distance = sq_dist;
                        min_diff = diff;
                        pComponents = &resources.Array[iResource];
                    }
                }
            }
        }

        out_distance = glm::sqrt(min_sq_distance);
        out_direction = min_diff / out_distance;
        return pComponents;
    }

    void ResourceGeneratorBehavior::DestroyResource(ResourceComponents *pComponents, Scene *pScene) {

        auto& resources = m_SpawnedByRoom[pComponents->Room.x][pComponents->Room.y];

        for (uint32_t iResource = 0; iResource < resources.Count; ++iResource)
        {
            if (pComponents->EID1 == resources.Array[iResource].EID1) {
                auto entity1 = resources.Array[iResource].EID1;
                auto entity2 = resources.Array[iResource].EID2;
                resources.Array[iResource] = resources.Array[resources.Count - 1];
                resources.Count--;
                pScene->DestroyEntity(entity1);
                pScene->DestroyEntity(entity2);
                return;
            }
        }

    }

    void ResourceGeneratorBehavior::DestroyAllResources(Scene *pScene) {
        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                auto& ennemies = m_SpawnedByRoom[x_room][y_room];
                for (uint32_t i = 0; i < ennemies.Count; ++i) {
                    pScene->DestroyEntity(ennemies.Array[i].EID1);
                    pScene->DestroyEntity(ennemies.Array[i].EID2);
                }
                ennemies.Count = 0;
            }
        }
    }


    void ResourceGeneratorBehavior::UpdateVisibilityBasedOnDistance() {
        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                glm::ivec2 abs_diff = glm::abs(m_CurrentRoom - glm::ivec2{ x_room, y_room });
                const bool IS_NEIGHBOR = abs_diff.x <= 1 && abs_diff.y <= 1;
                auto& resources = m_SpawnedByRoom[x_room][y_room];
                for (uint32_t iResource = 0; iResource < resources.Count; ++iResource) {
                    if ( resources.Array[iResource].UseSprite1 )
                        resources.Array[iResource].pRenderer1->IsEnable = IS_NEIGHBOR;

                    if ( resources.Array[iResource].UseSprite2 )
                        resources.Array[iResource].pRenderer2->IsEnable = IS_NEIGHBOR;

                    resources.Array[iResource].pBehavior->SetEnabled(IS_NEIGHBOR);
                }
            }
        }
    }

    void ResourceGeneratorBehavior::CreateResource(glm::vec2 position, glm::ivec2 room, Scene *pScene) {
        if (m_SpawnedByRoom[room.x][room.y].Count >= MAX_RESOURCE_COUNT) return;

        auto resource = pScene->CreateEntityAs3D();

        auto rTransform = pScene->GetComponent<Transform3D>(resource);
        rTransform->SetParent(mp_Transform);
        rTransform->LocalTransform.SetPosition(position.x, 0.0f, position.y);
        rTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.5f, GPC_PI);

        auto rRenderer = pScene->AddComponent<Render3DComponent>(resource);
        rRenderer->pGeometry = GPC_MESH("Plane");

        auto rTweening = pScene->AddComponent<TweenComponent>(resource);

        auto rBehavior = pScene->AddBehavior<ResourceBehavior>(resource);

        auto leaves = pScene->CreateEntityAs3D();

        auto lTransform = pScene->GetComponent<Transform3D>(leaves);
        lTransform->SetParent(mp_Transform);
        lTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.5f, GPC_PI);
        lTransform->LocalTransform.SetPosition(glm::vec3(position.x, 0.0f, position.y) + lTransform->LocalTransform.GetUp() * 0.01f);

        auto lRenderer = pScene->AddComponent<Render3DComponent>(leaves);
        lRenderer->pGeometry = GPC_MESH("Plane");

        m_SpawnedByRoom[room.x][room.y].Array[m_SpawnedByRoom[room.x][room.y].Count] = {
            .Room = room,
            .EID1 = resource,
            .EID2 = leaves,
            .pTransform1 = rTransform,
            .pTransform2 = lTransform,
            .pRenderer1 = rRenderer,
            .pRenderer2 = lRenderer,
            .pBehavior = rBehavior,
            .pTweening = rTweening
       };

        rBehavior->pManager = this;
        rBehavior->pComponents = &m_SpawnedByRoom[room.x][room.y].Array[m_SpawnedByRoom[room.x][room.y].Count];

        constexpr uint16_t RESOURCE_COUNT = 8;
        constexpr float Weights[RESOURCE_COUNT] = {
            1.0f,
            1.0f,
            0.01f,
            0.01f,
            0.01f,
            0.01f,
            0.01f,
            0.025f,
        };

        constexpr ResourceType Types[RESOURCE_COUNT] {
            ResourceType::WOOD,
            ResourceType::STONE,
            ResourceType::GREEN_GEMME,
            ResourceType::YELLOW_GEMME,
            ResourceType::RED_GEMME,
            ResourceType::BLUE_GEMME,
            ResourceType::ORANGE_GEMME,
            ResourceType::PURPLE_GEMME,
        };

        ResourceType resourceType = Types[GPC::Random::WeightedDistribution(Weights, RESOURCE_COUNT)];

        rBehavior->InitAs(1, resourceType, pScene);
        m_SpawnedByRoom[room.x][room.y].Count++;
    }
} // GPC