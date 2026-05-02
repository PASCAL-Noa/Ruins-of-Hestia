#include "RoomEditor/RoomEditorBehavior.h"

#include "FreeCamera.h"
#include "Scene.h"

namespace GPC {
    void RoomEditorBehavior::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);

        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());

        for (uint32_t i = 0; i < ROOM_SIZE; ++i) {
            for (uint32_t j = 0; j < ROOM_SIZE; ++j) {
                CreateTile({i, j}, pCtx->pScene);
            }
        }

        CreateBorder({ -1.0f,  static_cast<float>(ROOM_SIZE - 1) * 0.5f }, { 1, ROOM_SIZE }, mp_BorderLeftRenderer, pCtx->pScene);
        CreateBorder({ ROOM_SIZE, static_cast<float>(ROOM_SIZE - 1) * 0.5f }, { 1, ROOM_SIZE }, mp_BorderRightRenderer, pCtx->pScene);
        CreateBorder({  static_cast<float>(ROOM_SIZE - 1) * 0.5f, -1.0f  }, { ROOM_SIZE, 1 }, mp_BorderUpRenderer, pCtx->pScene);
        CreateBorder({  static_cast<float>(ROOM_SIZE - 1) * 0.5f, ROOM_SIZE,  }, { ROOM_SIZE, 1 }, mp_BorderDownRenderer, pCtx->pScene);

        CreateSelector(pCtx->pScene);
        CreateCamera(pCtx->pScene);

        EntityID id = CreatePlayerSpawner({0.5f * static_cast<float>(ROOM_SIZE), 0.5f * static_cast<float>(ROOM_SIZE) }, mp_Transform, pCtx->pScene);
        m_MetaRoom.ReplacePlayerSpawn(id, {0.5f * static_cast<float>(ROOM_SIZE), 0.5f * static_cast<float>(ROOM_SIZE) });
    }

    void RoomEditorBehavior::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        UpdateTileVisual();
        UpdateBorderVisual();
        UpdateMode();

        Ray ray{ mp_CameraTransform->GetWorldPosition(), -mp_CameraTransform->LocalTransform.GetForward() };
        RayHit hit;
        bool has_hit = pCtx->pScene->RayCast3D(hit, ray);
        if (has_hit) {
            m_Selector.Position = { (hit.ContactPoint.x + 0.5f), (hit.ContactPoint.z + 0.5f) };
            if (m_Selector.Position.x < 0) m_Selector.Position.x = 0;
            if (m_Selector.Position.x >= ROOM_SIZE) m_Selector.Position.x = ROOM_SIZE - 1;
            if (m_Selector.Position.y < 0) m_Selector.Position.y = 0;
            if (m_Selector.Position.y >= ROOM_SIZE) m_Selector.Position.y = ROOM_SIZE - 1;
            m_Selector.pPointerTransform->LocalTransform.SetPosition(hit.ContactPoint);
            m_Selector.pSelectorTransform->LocalTransform.SetPosition(
                m_Selector.Position.x,
                0.01f,
                m_Selector.Position.y
            );
        }

        if (has_hit && Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_MIDDLE)) {
            DestroySpawner(hit.HitEntity, pCtx->pScene);
        }

        HandleSaveLoadShortcuts(pCtx->pScene);

        switch (m_CurrentMode) {
            default:
            case BRUSH:
                if (Inputs::IsButtonDown(Inputs::ButtonCode::BUTTON_RIGHT))
                    DrawAt(m_Selector.Position);
                break;
            case FILL:
                if (Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_RIGHT))
                    FillAt(m_Selector.Position);
                break;
            case ERASER:
                if (Inputs::IsButtonDown(Inputs::ButtonCode::BUTTON_RIGHT))
                    EraseAt(m_Selector.Position);
                break;
            case ERASE_FILL:
                if (Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_RIGHT))
                    ClearAt(m_Selector.Position);
                break;
            case ENNEMI_SPAWN:
                if (has_hit && Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_RIGHT)) {
                    AddEnnemi({ hit.ContactPoint.x, hit.ContactPoint.z }, pCtx->pScene);
                }
                break;
            case RESOURCE_SPAWN:
                if (has_hit && Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_RIGHT)) {
                    AddResource({ hit.ContactPoint.x, hit.ContactPoint.z }, pCtx->pScene);
                }
                break;
            case PLAYER_SPAWN:
                if (has_hit && Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_RIGHT)) {
                    SetPlayerSpawn({ hit.ContactPoint.x, hit.ContactPoint.z }, pCtx->pScene);
                }
                break;
        }

        if (Inputs::IsKeyPress(Inputs::KeyCode::Q)) {
            pPreview->GenerateWith(&m_MetaRoom, pCtx->pScene);
        }
    }

    void RoomEditorBehavior::GoToEditMode() {
        mp_CameraComponent->Active = true;
        pPreview->SetEnabled(false);
        SetEnabled(true);
    }

    void RoomEditorBehavior::CreateTile(glm::ivec2 pos, Scene *pScene) {
        uint8_t active = TILE_EMPTY;

        auto tile = pScene->CreateEntityAs3D();

        auto tTransform =  pScene->GetComponent<Transform3D>(tile);
        auto tRenderer =  pScene->AddComponent<Render3DComponent>(tile);
        tTransform->SetParent(mp_Transform);
        tTransform->LocalTransform.SetPosition(pos.x, 0, pos.y);

        tRenderer->pGeometry = GPC_MESH("Plane");
        tRenderer->SetTexture("1024x1024", "Seamless1");

        tRenderer->IsEnable = active;

        m_MetaRoom.GetRaw().Tiles[pos.x][pos.y] = active;
        m_DebugGrid[pos.x][pos.y] = {
            .pTileTransform = tTransform,
            .pTileRenderer = tRenderer,
        };
    }

    void RoomEditorBehavior::CreateBorder(glm::vec2 pos, glm::vec2 scale, Render3DComponent*& outRenderer, Scene *pScene) {
        auto border = pScene->CreateEntityAs3D();
        Transform3D* bTransform =  pScene->GetComponent<Transform3D>(border);
        outRenderer =  pScene->AddComponent<Render3DComponent>(border);
        outRenderer->pGeometry = GPC_MESH("Plane");
        outRenderer->SetTexture("1024x1024", "Blue");
        bTransform->SetParent(mp_Transform);
        bTransform->LocalTransform.SetPosition({ pos.x, 0, pos.y });
        bTransform->LocalTransform.SetScale({ scale.x, 1, scale.y });
    }

    void RoomEditorBehavior::CreateCamera(Scene *pScene) {
        auto camera = pScene->CreateEntityAs3D();

        mp_CameraTransform = pScene->GetComponent<Transform3D>(camera);
        mp_CameraTransform->SetParent(mp_Transform);
        mp_CameraTransform->LocalTransform.SetPosition(ROOM_SIZE * 0.5f, 3.0f, ROOM_SIZE * 1.5f);

        mp_CameraComponent = pScene->AddComponent<CameraComponent>(camera);

        auto freeCamera = pScene->AddBehavior<FreeCamera>(camera);
        freeCamera->MaxAnglePitch = GPC_PI * 0.45f;
        freeCamera->MinAnglePitch = -GPC_PI * 0.45f;
    }

    void RoomEditorBehavior::CreateSelector(Scene *pScene) {

        // RAY CAST PLANE :
        m_Selector.RayCastPlaneID = pScene->CreateEntityAs3D();

        auto rcpTransform = pScene->GetComponent<Transform3D>( m_Selector.RayCastPlaneID);
        rcpTransform->SetParent(mp_Transform);
        rcpTransform->LocalTransform.SetScale(ROOM_SIZE * 2.0f, 0.001f, ROOM_SIZE * 2.0f);
        rcpTransform->LocalTransform.SetPosition( static_cast<float>(ROOM_SIZE) * 0.5f, 0, static_cast<float>(ROOM_SIZE) * 0.5f);

        auto rcpCollider =  pScene->AddComponent<Collision3DComponent>( m_Selector.RayCastPlaneID);
        rcpCollider->IsTrigger = true;
        rcpCollider->IsSphere = false;

        // POINTER :
        auto pointer =  pScene->CreateEntityAs3D();

        m_Selector.pPointerTransform = pScene->GetComponent<Transform3D>(pointer);
        m_Selector.pPointerTransform->SetParent(mp_Transform);
        m_Selector.pPointerTransform->LocalTransform.SetScale(0.2f, 0.2f, 0.2f);

        auto pRenderer =  pScene->AddComponent<Render3DComponent>(pointer);
        pRenderer->pGeometry = GPC_MESH("Sphere");
        pRenderer->SetTexture("1024x1024", "Red");

        // SELECTOR :
        auto selector =  pScene->CreateEntityAs3D();

        m_Selector.pSelectorTransform =  pScene->GetComponent<Transform3D>(selector);
        m_Selector.pSelectorTransform->SetParent(mp_Transform);
        m_Selector.pSelectorTransform->LocalTransform.SetPosition(0, 0.05f, 0);

        auto sRenderer =  pScene->AddComponent<Render3DComponent>(selector);
        sRenderer->pGeometry = GPC_MESH("Plane");
        sRenderer->SetTexture("1024x1024", "Debug2 Rotated");

    }


    EntityID RoomEditorBehavior::CreateEnnemiSpawner(glm::vec2 pos, Transform3D* pParent, Scene *pScene) {
        auto ennemi_spawn = pScene->CreateEntityAs3D();

        auto* spawnTransform = pScene->GetComponent<Transform3D>(ennemi_spawn);
        spawnTransform->SetParent(pParent);
        spawnTransform->LocalTransform.SetPosition( pos.x, 0.25f, pos.y );
        spawnTransform->LocalTransform.SetScale(ENNEMI_SPAWN_EDITOR_SCALE);

        auto* spawnCollider = pScene->AddComponent<Collision3DComponent>(ennemi_spawn);
        spawnCollider->IsSphere = true;
        spawnCollider->IsTrigger = true;

        auto* spawnRenderer = pScene->AddComponent<Render3DComponent>(ennemi_spawn);
        spawnRenderer->pGeometry = GPC_MESH("Sphere");
        spawnRenderer->SetTexture("1024x1024", "Red");

        return ennemi_spawn;
    }

    EntityID RoomEditorBehavior::CreateResourceSpawner(glm::vec2 pos, Transform3D* pParent, Scene *pScene) {
        auto resource_spawn = pScene->CreateEntityAs3D();

        auto* spawnTransform = pScene->GetComponent<Transform3D>(resource_spawn);
        spawnTransform->SetParent(pParent);
        spawnTransform->LocalTransform.SetPosition( pos.x, RESOURCE_SPAWN_EDITOR_SCALE.z * 0.5f, pos.y );
        spawnTransform->LocalTransform.SetScale(RESOURCE_SPAWN_EDITOR_SCALE);
        spawnTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.5f, GPC_PI);

        auto* spawnCollider = pScene->AddComponent<Collision3DComponent>(resource_spawn);
        spawnCollider->IsSphere = true;
        spawnCollider->IsTrigger = true;

        auto* spawnRenderer = pScene->AddComponent<Render3DComponent>(resource_spawn);
        spawnRenderer->pGeometry = GPC_MESH("PlaneDS");
        spawnRenderer->SetTexture("1024x1024", "Tree");

        return resource_spawn;
    }

    EntityID RoomEditorBehavior::CreatePlayerSpawner(glm::vec2 pos, Transform3D* pParent, Scene *pScene) {
        auto player_spawn = pScene->CreateEntityAs3D();

        auto* spawnTransform = pScene->GetComponent<Transform3D>(player_spawn);
        spawnTransform->SetParent(pParent);
        spawnTransform->LocalTransform.SetPosition( pos.x, 0.25f, pos.y );
        spawnTransform->LocalTransform.SetScale(PLAYER_SPAWN_EDITOR_SCALE);

        auto* spawnRenderer = pScene->AddComponent<Render3DComponent>(player_spawn);
        spawnRenderer->pGeometry = GPC_MESH("Sphere");
        spawnRenderer->SetTexture("1024x1024", "Blue");

        return player_spawn;
    }

    void RoomEditorBehavior::UpdateTileVisual() {
        for (uint32_t x = 0; x < ROOM_SIZE; ++x) {
            for (uint32_t y = 0; y < ROOM_SIZE; ++y) {
                if (m_MetaRoom.GetRaw().Tiles[x][y] != TILE_EMPTY) {
                    m_DebugGrid[x][y].pTileRenderer->IsEnable = true;
                } else {
                    m_DebugGrid[x][y].pTileRenderer->IsEnable = false;
                }
            }
        }
    }

    void RoomEditorBehavior::UpdateBorderVisual() {
        bool IS_UP_LINKED = false;
        bool IS_DOWN_LINKED = false;
        bool IS_LEFT_LINKED = false;
        bool IS_RIGHT_LINKED = false;

        for (uint32_t i = 0; i < ROOM_SIZE; ++i) {
            if ( m_MetaRoom.GetRaw().Tiles[i][0] != TILE_EMPTY ) IS_UP_LINKED = true;
            if ( m_MetaRoom.GetRaw().Tiles[0][i] != TILE_EMPTY ) IS_LEFT_LINKED = true;
            if ( m_MetaRoom.GetRaw().Tiles[i][ROOM_SIZE - 1] != TILE_EMPTY ) IS_DOWN_LINKED = true;
            if ( m_MetaRoom.GetRaw().Tiles[ROOM_SIZE - 1][i] != TILE_EMPTY ) IS_RIGHT_LINKED = true;
        }

        mp_BorderUpRenderer->SetTexture("1024x1024", IS_UP_LINKED ? "Blue" : "Red");
        mp_BorderDownRenderer->SetTexture("1024x1024", IS_DOWN_LINKED ? "Blue" : "Red");
        mp_BorderLeftRenderer->SetTexture("1024x1024", IS_LEFT_LINKED ? "Blue" : "Red");
        mp_BorderRightRenderer->SetTexture("1024x1024", IS_RIGHT_LINKED ? "Blue" : "Red");
    }

    void RoomEditorBehavior::UpdateMode() {
        if (Inputs::IsKeyPress(Inputs::KeyCode::F1)) {
            m_CurrentMode = BRUSH;
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::F2)) {
            m_CurrentMode = FILL;
        }

        if (Inputs::IsKeyPress(Inputs::KeyCode::F3)) {
            m_CurrentMode = ERASER;
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::F4)) {
            m_CurrentMode = ERASE_FILL;
        }

        if (Inputs::IsKeyPress(Inputs::KeyCode::F5)) {
            m_CurrentMode = ENNEMI_SPAWN;
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::F6)) {
            m_CurrentMode = RESOURCE_SPAWN;
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::F7)) {
            m_CurrentMode = PLAYER_SPAWN;
        }
    }

    void RoomEditorBehavior::HandleSaveLoadShortcuts(Scene* pScene) {
        if (Inputs::IsKeyPress(Inputs::KeyCode::LEFT_ALT)) {
            m_SlotMode      = SlotMode::Save;
            m_SlotBuffer    = 0;
            m_SlotHasDigit  = false;
            m_SlotCancelled = false;
            GPC_INFO << "[Editor] Save slot input started (type digits, release Left Alt to save)" << ENDL;
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::RIGHT_ALT)) {
            m_SlotMode      = SlotMode::Load;
            m_SlotBuffer    = 0;
            m_SlotHasDigit  = false;
            m_SlotCancelled = false;
            GPC_INFO << "[Editor] Load slot input started (type digits, release Right Alt to load)" << ENDL;
        }

        if (m_SlotMode == SlotMode::None) return;

        if (Inputs::IsKeyPress(Inputs::KeyCode::ESCAPE)) {
            GPC_INFO << "[Editor] Slot input cancelled" << ENDL;
            m_SlotMode      = SlotMode::None;
            m_SlotBuffer    = 0;
            m_SlotHasDigit  = false;
            m_SlotCancelled = false;
            return;
        }

        const char* modeLabel = (m_SlotMode == SlotMode::Save) ? "Save" : "Load";

        for (uint32_t d = 0; d < 10; ++d) {
            const auto kpKey  = static_cast<Inputs::KeyCode>(static_cast<int>(Inputs::KeyCode::KP_0)  + d);
            const auto topKey = static_cast<Inputs::KeyCode>(static_cast<int>(Inputs::KeyCode::KEY_0) + d);
            const bool digitPressed = Inputs::IsKeyPress(kpKey) || Inputs::IsKeyPress(topKey);
            if (!digitPressed) continue;

            const uint32_t next = m_SlotBuffer * 10 + d;
            if (next > MAX_SLOT_INDEX) continue;

            m_SlotBuffer   = next;
            m_SlotHasDigit = true;
            GPC_INFO << "[Editor] " << modeLabel << " slot: " << m_SlotBuffer << ENDL;
        }

        if (Inputs::IsKeyPress(Inputs::KeyCode::BACKSPACE) || Inputs::IsKeyPress(Inputs::KeyCode::KP_SUBTRACT)) {
            m_SlotBuffer /= 10;
            if (m_SlotBuffer == 0) m_SlotHasDigit = false;
            GPC_INFO << "[Editor] " << modeLabel << " slot: " << m_SlotBuffer << ENDL;
        }

        const bool altReleased   = (m_SlotMode == SlotMode::Save && Inputs::IsKeyRelease(Inputs::KeyCode::LEFT_ALT))
                                || (m_SlotMode == SlotMode::Load && Inputs::IsKeyRelease(Inputs::KeyCode::RIGHT_ALT));
        const bool enterPressed  = Inputs::IsKeyPress(Inputs::KeyCode::ENTER) || Inputs::IsKeyPress(Inputs::KeyCode::KP_ENTER);

        if (!altReleased && !enterPressed) return;

        if (m_SlotHasDigit && !m_SlotCancelled) {
            if (m_SlotMode == SlotMode::Save) SaveToFile(m_SlotBuffer);
            else                              LoadFromFile(m_SlotBuffer, pScene);
        }

        m_SlotMode      = SlotMode::None;
        m_SlotBuffer    = 0;
        m_SlotHasDigit  = false;
        m_SlotCancelled = false;
    }

    void RoomEditorBehavior::SaveToFile(uint32_t fileIndex) {
        std::string file = "Resources/RoomData/Room" + std::to_string(fileIndex) + ".room";
        m_MetaRoom.SaveToFile(file);
        GPC_INFO << "Saved into : " << file << ENDL;
    }

    void RoomEditorBehavior::LoadFromFile(uint32_t fileIndex, Scene* pScene) {
        std::string file = "Resources/RoomData/Room" + std::to_string(fileIndex) + ".room";
        m_MetaRoom.LoadFromFile(file);
        ReGenerateAllSpawnerFromMeta(pScene);
        GPC_INFO << "Loaded from : " << file << ENDL;
    }

    void RoomEditorBehavior::ReGenerateAllSpawnerFromMeta(Scene *pScene) {
        for (auto spawner : m_LivingSpawners) {
            pScene->DestroyEntity(spawner);
        }
        m_LivingSpawners.clear();

        m_MetaRoom.GetEnnemiesID().resize(m_MetaRoom.GetEnnemies().size());
        for (uint32_t i = 0; i < m_MetaRoom.GetEnnemies().size(); ++i) {
            EntityID id = CreateEnnemiSpawner(m_MetaRoom.GetEnnemies()[i].position, mp_Transform, pScene);
            m_LivingSpawners.push_back(id);
            m_MetaRoom.GetEnnemiesID()[i] = id;
        }

        m_MetaRoom.GetResourcesID().resize(m_MetaRoom.GetResources().size());
        for (uint32_t i = 0; i < m_MetaRoom.GetResources().size(); ++i) {
            EntityID id = CreateResourceSpawner(m_MetaRoom.GetResources()[i].position, mp_Transform, pScene);
            m_LivingSpawners.push_back(id);
            m_MetaRoom.GetResourcesID()[i] = id;
        }

        SetPlayerSpawn(m_MetaRoom.GetPlayer().position, pScene);
    }

    void RoomEditorBehavior::DrawAt(glm::ivec2 pos) {
        m_MetaRoom.GetRaw().Tiles[pos.x][pos.y] = 1;
    }

    void RoomEditorBehavior::FillAt(glm::ivec2 pos) {
        static glm::ivec2 TO_CHECK[4] = { { -1, 0 }, { 1, 0 }, { 0, -1}, { 0, 1 } };
        set_ivec2 seen{};
        std::queue<glm::ivec2> next{};
        next.push(m_Selector.Position);
        while (next.empty() == false) {
            glm::ivec2 current = next.front();
            m_MetaRoom.GetRaw().Tiles[current.x][current.y] = 1;
            for (uint32_t i = 0; i < 4; ++i) {
                glm::ivec2 new_tile = current + TO_CHECK[i];
                if (
                    new_tile.x >= 0
                    && new_tile.y >= 0
                    && new_tile.x < ROOM_SIZE
                    && new_tile.y < ROOM_SIZE
                    && m_MetaRoom.GetRaw().Tiles[new_tile.x][new_tile.y] == TILE_EMPTY
                    && seen.contains(new_tile) == false)
                {
                    next.push(new_tile);
                }
            }
            seen.insert(current);
            next.pop();
        }
    }

    void RoomEditorBehavior::EraseAt(glm::ivec2 pos) {
        m_MetaRoom.GetRaw().Tiles[pos.x][pos.y] = TILE_EMPTY;
    }

    void RoomEditorBehavior::ClearAt(glm::ivec2 pos) {
        static glm::ivec2 TO_CHECK[4] = { { -1, 0 }, { 1, 0 }, { 0, -1}, { 0, 1 } };
        set_ivec2 seen{};
        std::queue<glm::ivec2> next{};
        next.push(m_Selector.Position);
        while (next.empty() == false) {
            glm::ivec2 current = next.front();
            m_MetaRoom.GetRaw().Tiles[current.x][current.y] = TILE_EMPTY;
            for (uint32_t i = 0; i < 4; ++i) {
                glm::ivec2 new_tile = current + TO_CHECK[i];
                if (
                    new_tile.x >= 0
                    && new_tile.y >= 0
                    && new_tile.x < ROOM_SIZE
                    && new_tile.y < ROOM_SIZE
                    && m_MetaRoom.GetRaw().Tiles[new_tile.x][new_tile.y] != TILE_EMPTY
                    && seen.contains(new_tile) == false)
                {
                    next.push(new_tile);
                }
            }
            seen.insert(current);
            next.pop();
        }
    }

    void RoomEditorBehavior::AddEnnemi(glm::vec2 pos, Scene* pScene) {
        if (pos.x < 0.0f || pos.y < 0.0f || pos.x > (ROOM_SIZE - 1) || pos.y > (ROOM_SIZE - 1)) return;

        EntityID id = CreateEnnemiSpawner(pos, mp_Transform, pScene);
        m_LivingSpawners.push_back(id);
        m_MetaRoom.AddEnnemiSpawnByID(id, pos);
    }

    void RoomEditorBehavior::AddResource(glm::vec2 pos, Scene* pScene) {
        if (pos.x < 0.0f || pos.y < 0.0f || pos.x > (ROOM_SIZE - 1) || pos.y > (ROOM_SIZE - 1)) return;
        EntityID id = CreateResourceSpawner(pos, mp_Transform, pScene);
        m_LivingSpawners.push_back(id);
        m_MetaRoom.AddResourceSpawnByID(id, pos);
    }

    void RoomEditorBehavior::SetPlayerSpawn(glm::vec2 pos, Scene* pScene) {
        if (pos.x < 0.0f || pos.y < 0.0f || pos.x > (ROOM_SIZE - 1) || pos.y > (ROOM_SIZE - 1)) return;
        EntityID id = CreatePlayerSpawner(pos, mp_Transform, pScene);
        pScene->DestroyEntity(m_MetaRoom.ReplacePlayerSpawn(id, pos));
    }

    void RoomEditorBehavior::DestroySpawner(EntityID id, Scene *pScene) {
        for (auto it = m_LivingSpawners.begin(); it != m_LivingSpawners.end(); ++it) {
            if (*it == id) {
                m_LivingSpawners.erase(it);
                m_MetaRoom.RemoveEnnemiSpawnByID(id);
                m_MetaRoom.RemoveResourceSpawnByID(id);
                pScene->DestroyEntity(id);
                return;
            }
        }
    }
} // GPC