#include "Map Generation/TerrainGeneratorBehavior.h"


#include "ArchiveManager.h"
#include "Assets.h"
#include "LightingSystem.h"
#include "Random.h"
#include "Render3DSystem.h"
#include "Scene.h"
#include "Map Generation/MapGenerator.h"
#include "RoomEditor/RoomEditorBehavior.h"
#include "Scenes/SceneDefault.h"

namespace GPC {
    void TerrainGeneratorBehavior::CreateBorder(const GPC::MapGeneratorData::MapBorder &border, Scene *pScene) {
        auto eBorder = pScene->CreateEntityAs3D();
        auto bTransform = pScene->GetComponent<Transform3D>(eBorder);
        auto bRenderer = pScene->AddComponent<Render3DComponent>(eBorder);
        auto bCollider = pScene->AddComponent<Collision3DComponent>(eBorder);

        constexpr float RESIZE_COEF = GPC::MapGeneratorData::TILE_GROUP_SCALE * 0.5f;
        glm::vec2 pos = border.Corner + border.Size * 0.5f;
        bTransform->LocalTransform.SetPosition(pos.x * RESIZE_COEF, BORDER_HEIGHT * 0.5f, pos.y * RESIZE_COEF);
        bTransform->LocalTransform.SetScale(border.Size.x * RESIZE_COEF, BORDER_HEIGHT, border.Size.y * RESIZE_COEF);

        bRenderer->pGeometry = GPC_MESH("Cube");
        bRenderer->SetTexture("1024x1024", "Blue");
        bRenderer->IsEnable = false;

        bCollider->IsSphere = false;
        bCollider->IsTrigger = false;
        bCollider->IsKinematic = false;
        bCollider->CollType = ColliderType::MapBorder;

        mp_CollisionRenderer.push_back(bRenderer);
    }

    void TerrainGeneratorBehavior::CreateRoom(RoomData &room_data, Geometry *pGeometry, Scene *pScene) {
        auto room = pScene->CreateEntityAs3D();
        auto* transform = pScene->GetComponent<Transform3D>(room);
        auto* renderer = pScene->AddComponent<Render3DComponent>(room);

        transform->SetParent(mp_Transform);
        transform->LocalTransform.SetPosition(room_data.Position);
        transform->LocalTransform.SetScale(room_data.Scale);

        renderer->pGeometry = pGeometry;
        renderer->SetTexture("1024x1024", "Seamless1");
        renderer->UseShadow = false;
        renderer->pGraphicProgram = &GPC_GRAPHIC_PROGRAM("Map Program")->Program;

        m_RoomComponents[room_data.MapPosition.x][room_data.MapPosition.y].pTransform = transform;
        m_RoomComponents[room_data.MapPosition.x][room_data.MapPosition.y].pRenderer = renderer;
    }

    void TerrainGeneratorBehavior::CreateBushes(Scene *pScene) {

        auto pGeneration = MapGeneratorFactory::GetGeneration(GenerationID);

        for (uint32_t i_prop = 0; i_prop < PROP_COUNT; ++i_prop) {

            if ((*pProps)[i_prop] == nullptr) {
                auto batch = pScene->CreateEntityAs3D();
                (*pProps)[i_prop] = pScene->AddComponent<Batching3DComponent>(batch);
                (*pProps)[i_prop]->pGeometry = GPC_MESH("Plane");
                (*pProps)[i_prop]->pGraphicProgram = &GPC_GRAPHIC_PROGRAM("Props" + std::to_string(i_prop) + " Program")->Program;
                (*pProps)[i_prop]->SetTexture("Props", "Bush" + std::to_string(i_prop));
                (*pProps)[i_prop]->Batched3DObjects.resize(PROP_COUNT_BY_TYPE);
            }

            for (auto& batched : (*pProps)[i_prop]->Batched3DObjects) {


                uint32_t room_x = rand() % MAX_MAP_SIZE;
                uint32_t room_y = rand() % MAX_MAP_SIZE;
                uint32_t tile_x;
                uint32_t tile_y;
                uint32_t sub_tile_x;
                uint32_t sub_tile_y;

                do {
                    room_x = rand() % MAX_MAP_SIZE;
                    room_y = rand() % MAX_MAP_SIZE;
                    if (pGeneration->GeneratedMapMask.Get(room_x, room_y) == false) continue;
                    tile_x = rand() % TILE_GROUP_COUNT;
                    tile_y = rand() % TILE_GROUP_COUNT;
                    sub_tile_x = rand() % 2;
                    sub_tile_y = rand() % 2;
                    bool tile = pGeneration->GeneratedTileGroupLayout[room_x][room_y][tile_x][tile_y].b[sub_tile_x][sub_tile_y] != TILE_EMPTY;
                    if (tile) {
                        break;
                    }
                } while (true);

                float X = MapGeneratorData::ROOM_SCALE * room_x + MapGeneratorData::TILE_GROUP_SCALE * (tile_x * 2 + sub_tile_x) * 0.5f;
                float Y = MapGeneratorData::ROOM_SCALE * room_y + MapGeneratorData::TILE_GROUP_SCALE * (tile_y * 2 + sub_tile_y) * 0.5f;

                float scale = GPC::Random::Float(0.25f, 0.75f);
                batched.Size = {
                    scale * GENERAL_SCALE_EXPEDITION * 2.0f,
                    1.0f,
                    scale * GENERAL_SCALE_EXPEDITION * 1.4f,
                    1.0f
                };

                batched.Position = {
                    X,
                    batched.Size.z * 0.5f,
                    Y,
                    rand() % 16
                };

                batched.Rotation = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.5f, GPC_PI);

                batched.Tint = Colors::WHITE;
            }
        }

    }

    void TerrainGeneratorBehavior::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);

        auto pGeneration = MapGeneratorFactory::GetGeneration(GenerationID);

        for (auto& border : pGeneration->GeneratedBorders) {
            CreateBorder(border, pCtx->pScene);
        }

        for (auto room : pGeneration->GeneratedRoomDatas) {
            CreateRoom(room, room.pGeometry, pCtx->pScene);
        }

        CreateBushes(pCtx->pScene);
        m_IsGenerated = true;
    }

    void TerrainGeneratorBehavior::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);
        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
    }

    void TerrainGeneratorBehavior::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (Inputs::IsKeyPress(Inputs::KeyCode::Q)) {
            for (auto renderer : mp_CollisionRenderer) {
                renderer->IsEnable = ! renderer->IsEnable;
            }
        }

        if (pPlayerTransform == nullptr) return;

        glm::ivec2 currentRoom = WorldPositionToRoomPosition(pPlayerTransform->LocalTransform.GetPosition());
        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                if (m_RoomComponents[x_room][y_room].pRenderer == nullptr) continue;
                glm::ivec2 abs_diff = glm::abs(currentRoom - glm::ivec2{ x_room, y_room });
                const bool IS_NEIGHBOR = abs_diff.x <= 1 && abs_diff.y <= 1;
                m_RoomComponents[x_room][y_room].pRenderer->IsEnable = IS_NEIGHBOR;
            }
        }
    }

    void TerrainGeneratorBehavior::OnDestroy() {
        Behavior::OnDestroy();
    }

    void TerrainGeneratorBehavior::DestroyTerrain(Scene* pScene) const {

        for (uint16_t xRoom = 0; xRoom < MAX_MAP_SIZE; ++xRoom) {
            for (uint16_t yRoom = 0; yRoom < MAX_MAP_SIZE; ++yRoom) {
                if (m_RoomComponents[xRoom][yRoom].pTransform != nullptr) {
                    pScene->DestroyEntity(m_RoomComponents[xRoom][yRoom].pRenderer->GetEntityID());
                }
            }
        }

        for (auto borders : mp_CollisionRenderer) {
            pScene->DestroyEntity(borders->GetEntityID());
        }

    }
}

 // GPC