#include "Village/Map/Editor.h"
#include "Village/Map/Map.h"
#include "Village/VillageManager.h"
#include "Scene.h"
#include "Inputs.h"
#include "UI/UiAnim.h"
#include "UI/UiBuilder.h"
#include "UI/UiAnimFluent.h"

namespace GPC
{
    void Editor::OnCreate(const BehaviorCreateContext *pCtx)
    {
        Behavior::OnCreate(pCtx);
        m_pScene = pCtx->pScene;

        m_pTransform = m_pScene->GetComponent<Transform3D>(GetEntityID());
        m_pRender3D = m_pScene->GetComponent<Render3DComponent>(GetEntityID());

        m_GhostEntity = m_pScene->CreateEntityAs3D();
        m_pGhostTransform = m_pScene->GetComponent<Transform3D>(m_GhostEntity);
        m_pGhostRender = m_pScene->AddComponent<Render3DComponent>(m_GhostEntity);
        m_pGhostRender->IsEnable = false;

        m_RemoveMode = false;
        m_CurrentTypeIdx = 1;
        m_SelectorGridPos = glm::vec3(0.0f);
        m_pMap = nullptr;
        m_pVillageManager = nullptr;

        m_LastTypeIdx = BuildingType::Count;
        m_LastStockState = false;

        CreateUI();
        mp_UiSystem = m_pScene->GetSystem<UiSystem>();


    }

    void Editor::OnDestroy()
    {
        if (m_GhostEntity != 0 && m_pScene)
            m_pScene->DestroyEntity(m_GhostEntity);
    }

    void Editor::SetMap(Map* pMap) { m_pMap = pMap; }
    void Editor::SetVillageManager(VillageManager* pVillageManager) { m_pVillageManager = pVillageManager; }

    void Editor::SetPlannerActive(bool active)
    {
        m_IsActive = active;
        if (m_pRender3D) m_pRender3D->IsEnable = active;
        if (m_pGhostRender) m_pGhostRender->IsEnable = active;
    }

    void Editor::SetFullUIActive(bool active)
    {
        m_EditorCanvas->IsEnable = active;
    }

    void Editor::SetCompactUIActive(bool active)
    {

    }

    void Editor::SetCursorAt(glm::vec3 pos) { m_SelectorGridPos = pos; }

    void Editor::CreateUI()
    {

        UiBuilder builder(m_pScene);

        m_EditorCanvas = builder
            .At(Anchors::BOTTOM_MIDDLE, { 0, 0 })
            .Size({ 1551, 277 })
            .Bg(Colors::APRICOT)
            .BuildCanvas();

        builder
            .At(Anchors::MIDDLE_MIDDLE, { 0, 225 })
            .Scale( {0.65, 0.65f} )
            .Sprite("1551x277", "EditMode_Panel")
            .ChildOf(m_EditorCanvas->GetEntityID())
            .Bg(Colors::APRICOT)
            .BuildSprite();

        const uint32_t STORAGE_COUNT = 5;
        for (int i = 0; i < STORAGE_COUNT; i++) {
            builder
                .At(Anchors::MIDDLE_MIDDLE, { -400 + 200 * i, 225 })
                .Scale( {0.75, 0.75f} )
                .Sprite("250x180", "EditMode_Building")
                .ChildOf(m_EditorCanvas->GetEntityID())
                .BuildSprite();
            m_EditorSprites.push_back(builder
                .At(Anchors::MIDDLE_MIDDLE, { -410 + 200 * i, 225 })
                .Scale( {0.75, 0.75f} )
                .Sprite("218x169", "Icon_Comming_Soon")
                .ChildOf(m_EditorCanvas->GetEntityID())
                .BuildSprite());
            builder
                .At(Anchors::MIDDLE_MIDDLE, { -320 + 200 * i, 166 })
                .Scale( {0.50, 0.50f} )
                .Sprite("250x180", "EditMode_Number")
                .ChildOf(m_EditorCanvas->GetEntityID())
                .BuildSprite();
            m_EditorTexts.push_back(builder
                .At(Anchors::MIDDLE_MIDDLE, { -320 + 200 * i, 170 })
                .Scale( {0.50, 0.50f} )
                .FontSize(42)
                .Text("0")
                .Tint(Colors::BLACK)
                .ChildOf(m_EditorCanvas->GetEntityID())
                .BuildText());
        }

        const float BASE_X      = 400;
        const float BASE_Y      = 0;
        const float BASE_GAP    = 100;
        m_SaveButtonSprite = builder
            .At(Anchors::MIDDLE_MIDDLE, { BASE_X, BASE_Y })
            .Scale( {0.80, 0.80f} )
            .Sprite("250x180", "EditMode_Save")
            .ChildOf(m_EditorCanvas->GetEntityID())
            .Bg(Colors::APRICOT)
            .BuildSprite();
        builder
            .At(Anchors::MIDDLE_MIDDLE, { 0, 5 })
            .FontSize(42)
            .Text("Sauvg.")
            .Tint(Colors::PERFECT_BROWN)
            .ChildOf(m_SaveButtonSprite->GetEntityID())
            .BuildText();

        UiButton* saveBtnLogic = builder.BuildButton(m_SaveButtonSprite);
        saveBtnLogic->OnClick.ConnectLambda([&] {
            if (m_BuildingCount > 0) {
                UiAnim::On(*m_pScene, m_SaveButtonSprite)
                    .TintTo(Colors::DARK_RED, 0.2f)
                    .After(0.2f)
                    .TintTo(Colors::PERFECT_BROWN, 0.2f);
                for (int i = 0; i < m_EditorSprites.size(); i++) {
                    auto type = static_cast<BuildingType>(i);
                    if (m_pVillageManager->GetStashedCount(type) > 0)
                        UiAnim::ScalePulse(*m_pScene, m_EditorSprites[i], 1.3f, 0.4f);
                }
                return;
            }
            SetPlacementMode(false);
            m_pVillageManager->GetVillageController()->EndEditing();
        });

        // UiSprite* cancelBtn = builder
        //     .At(Anchors::MIDDLE_MIDDLE, { BASE_X, BASE_Y + BASE_GAP*1.0f })
        //     .Scale( {0.80f, 0.80f} )
        //     .Sprite("250x180", "EditMode_Cancel")
        //     .ChildOf(m_EditorCanvas->GetEntityID())
        //     .Bg(Colors::APRICOT)
        //     .BuildSprite();
        // builder
        //     .At(Anchors::MIDDLE_MIDDLE, { 0, 5 })
        //     .FontSize(42)
        //     .Text("Annuler")
        //     .Tint(Colors::BLACK)
        //     .ChildOf(cancelBtn->GetEntityID())
        //     .BuildText();
        //
        // UiButton* cancelBtnLogic = builder.BuildButton(saveBtn);
        // cancelBtnLogic->OnClick.ConnectLambda([&] {
        //     m_RemoveMode = false;
        //     m_pVillageManager->GetVillageController()->EndEditing();
        // });

        UiSprite* clearBtn = builder
            .At(Anchors::MIDDLE_MIDDLE, { BASE_X, BASE_Y + BASE_GAP*1.0f })
            .Scale( {0.80, 0.80f} )
            .Sprite("250x180", "EditMode_Clear")
            .ChildOf(m_EditorCanvas->GetEntityID())
            .Bg(Colors::APRICOT)
            .BuildSprite();
        m_EditorPlacementText = builder
            .At(Anchors::MIDDLE_MIDDLE, { 5, 2 })
            .FontSize(42)
            .Text("Suppression")
            .Tint(Colors::PERFECT_BROWN)
            .ChildOf(clearBtn->GetEntityID())
            .BuildText();

        UiButton* clearAllBtn = builder.BuildButton(clearBtn);
        clearAllBtn->OnClick.ConnectLambda([&] {
            m_LastRemoveMode = m_RemoveMode;
            SetPlacementMode(!m_RemoveMode);
        });

        m_EditorCanvas->IsEnable = false;

    }

    void Editor::ClampCursorPos()
    {
        if (!m_pMap) return;
        glm::vec2 mapSize = m_pMap->GetSize();
        m_SelectorGridPos.x = glm::clamp(m_SelectorGridPos.x, 0.5f, mapSize.x - 0.5f);
        m_SelectorGridPos.y = glm::clamp(m_SelectorGridPos.y, 0.5f, mapSize.y - 0.5f);
    }

    glm::ivec2 Editor::GetCursor()
    {
        ClampCursorPos();
        return glm::ivec2(static_cast<int>(m_SelectorGridPos.x), static_cast<int>(m_SelectorGridPos.y));
    }

    void Editor::SetPlacementMode(bool mode)
    {
        m_RemoveMode = mode;
        if (mode) {
            m_EditorPlacementText->SetTint(Colors::RED);
        }
        else {
            m_EditorPlacementText->SetTint(Colors::BLACK);
        }
    }

    void Editor::OnUpdate(const BehaviorUpdateContext *pCtx)
    {
        Behavior::OnUpdate(pCtx);
        if (!m_IsActive || !m_pMap || !m_pVillageManager) return;

        RayHit hit = m_pScene->GetCameraRay({Inputs::GetMouseX(), Inputs::GetMouseY()});
        if (hit.ContactPoint.x < 1E10) {
            auto* camera = m_pScene->GetActiveCamera();
            auto* camTransform = m_pScene->GetComponent<Transform3D>(camera->GetEntityID());
            const glm::vec3 rayDir = -camTransform->LocalTransform.GetForward();
            const glm::vec2 worldXZ = m_pMap->ProjectRayToTiltedGround(hit.ContactPoint, rayDir);
            SetCursorAt({worldXZ.x, worldXZ.y, 0.0f});
        }

        if (m_pVillageManager->IsPlacingNew())
        {
            UpdateGhostVisuals();
            return;
        }

        constexpr int BUILDING_NUMBER = 3;
        for (int i = 0; i < BUILDING_NUMBER; i++) {

            auto type = static_cast<BuildingType>(i);
            auto build = m_pVillageManager->GetBuildingTemplate(type);

            m_EditorSprites[i]->SetSprite("218x169", build.Infos->Icon);
            m_EditorTexts[i]->SetText(std::to_string(m_pVillageManager->GetStashedCount(type)));

        }

        SelectorInputs(pCtx);
        ClampCursorPos();

        if (m_RemoveMode)
        {

            int gridX = static_cast<int>(m_SelectorGridPos.x);
            int gridY = static_cast<int>(m_SelectorGridPos.y);

            m_pTransform->LocalTransform.SetScale(glm::vec3(1.0f));
            const float cx = gridX + 0.5f;
            const float cz = gridY + 0.5f;
            m_pTransform->LocalTransform.SetPosition(cx, m_pMap->GetGroundYAt(cx, cz) + 0.05f, cz);
            m_pTransform->LocalTransform.SetRotationYawPitchRoll(
                glm::radians(VillageGroundTilt::YAW_DEG),
                glm::radians(VillageGroundTilt::PITCH_DEG),
                glm::radians(VillageGroundTilt::ROLL_DEG));
            m_pRender3D->pGeometry = GPC_MESH("Plane");
            m_pRender3D->RenderMaterial.Tint = Colors::RED;
            m_pGhostRender->IsEnable = false;

            if (Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_LEFT))
                if (!mp_UiSystem->HasPressedButton())
                    StoreBuilding();
        }
        else
        {
            UpdateGhostVisuals();

            if (Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_LEFT))
                if (!mp_UiSystem->HasPressedButton())
                    PlaceBuilding();
        }

    }

    void Editor::SelectorInputs(const BehaviorUpdateContext *pCtx)
    {
        if (Inputs::IsKeyPress(Inputs::KeyCode::TAB)) CycleToNextAvailable();
    }

    void Editor::CycleToNextAvailable()
    {
        int nextIdx = m_CurrentTypeIdx;
        bool found = false;

        for (int i = 1; i < static_cast<int>(BuildingType::Count); ++i)
        {
            nextIdx = (nextIdx + 1) % static_cast<int>(BuildingType::Count);
            BuildingType checkType = static_cast<BuildingType>(nextIdx);

            if (checkType != BuildingType::LifeTree && m_pVillageManager->GetStashedCount(checkType) > 0)
            {
                found = true;
                break;
            }
        }
        if (found)
            m_CurrentTypeIdx = nextIdx;
    }

    glm::ivec2 Editor::GetCursorGridPosition() const
    {
        return { static_cast<int>(m_SelectorGridPos.x), static_cast<int>(m_SelectorGridPos.y) };
    }

    bool Editor::IsCursorHoveringBuilding(const glm::ivec2& cursorPos, const BuildingData& building) const
    {
        glm::ivec2 size = m_pVillageManager->GetBuildingTemplate(building.Type).Size;
        bool isInsideX = (cursorPos.x >= building.Position.x) && (cursorPos.x < building.Position.x + size.x);
        bool isInsideY = (cursorPos.y >= building.Position.y) && (cursorPos.y < building.Position.y + size.y);
        return isInsideX && isInsideY;
    }

    Editor::GhostState Editor::GetCurrentGhostState() const
    {
        GhostState state;

        if (m_pVillageManager->IsPlacingNew())
        {
            state.Type = m_pVillageManager->GetPlacementType();
            state.HasStock = true;
        }
        else
        {
            state.Type = static_cast<BuildingType>(m_CurrentTypeIdx);
            state.HasStock = m_pVillageManager->GetStashedCount(state.Type) > 0;
        }

        if (state.HasStock)
        {
            const auto& templateData = m_pVillageManager->GetBuildingTemplate(state.Type);
            state.MeshPath = templateData.Path;
            state.Size = templateData.Size;
            state.Scale = templateData.Scale;
            state.Offset = templateData.Infos->Offset;
            state.SpriteDepthFactor   = templateData.Infos->SpriteDepthFactor;
            state.SpriteLateralFactor = templateData.Infos->SpriteLateralFactor;
        }
        else
        {
            state.MeshPath = "Plane";
            state.Size = { 1, 1 };
            state.Scale = 1.0f;
            state.SpriteDepthFactor   = VillageGroundTilt::SPRITE_DEPTH_FACTOR;
            state.SpriteLateralFactor = VillageGroundTilt::SPRITE_LATERAL_FACTOR;
        }

        return state;
    }

    void Editor::UpdateGhostGeometry(const GhostState& state)
    {
        if (m_LastRemoveMode != m_RemoveMode && (m_LastTypeIdx != state.Type || m_LastStockState != state.HasStock))
        {
            m_pRender3D->pGeometry = GPC_MESH("Cube");
            m_pRender3D->RenderMaterial.Diffuse = 0;

            m_pTransform->LocalTransform.SetScale(glm::vec3(state.Size.x * 0.9f, 0.05f, state.Size.y * 0.9f));

            if (state.HasStock && state.MeshPath != "Plane")
            {
                m_pGhostRender->IsEnable = true;
                m_pGhostRender->pGeometry = GPC_MESH("PlaneDS");
                m_pGhostRender->SetTexture("1220x1683", state.MeshPath);

                const float texW    = 1220.0f;
                const float texH    = 1683.0f;
                const float perspectiveRatio = 1.4f;

                float spriteW = static_cast<float>(state.Size.x);
                float spriteH = spriteW * (texH / texW) * perspectiveRatio;

                m_pGhostTransform->LocalTransform.SetScale({ spriteW * state.Scale, 1.0f, spriteH * state.Scale });
            }
            else
                m_pGhostRender->IsEnable = false;

            m_LastTypeIdx = state.Type;
            m_LastStockState = state.HasStock;
            m_LastRemoveMode = true;
        }
    }

    void Editor::UpdateGhostPosition(const GhostState& state) const
    {
        int gridX = static_cast<int>(m_SelectorGridPos.x);
        int gridY = static_cast<int>(m_SelectorGridPos.y);

        float offsetX = state.Size.x * 0.5f;
        float offsetY = state.Size.y * 0.5f;
        const float ghostCubeX = gridX + offsetX + state.Offset.x;
        const float ghostCubeZ = gridY + offsetY + state.Offset.y;
        m_pTransform->LocalTransform.SetPosition(ghostCubeX, m_pMap->GetGroundYAt(ghostCubeX, ghostCubeZ) + 0.025f, ghostCubeZ);
        m_pTransform->LocalTransform.SetRotationYawPitchRoll(
            glm::radians(VillageGroundTilt::YAW_DEG),
            glm::radians(VillageGroundTilt::PITCH_DEG),
            glm::radians(VillageGroundTilt::ROLL_DEG));

        if (m_pGhostRender->IsEnable)
        {
            m_pGhostTransform->LocalTransform.SetPosition(glm::vec3(gridX, m_pMap->GetGroundYAt((float)gridX, (float)gridY) + 1.0f, gridY));
            m_pGhostTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.5f, GPC_PI);
            m_pGhostTransform->LocalTransform.AddPosition(-m_pGhostTransform->LocalTransform.GetForward() * (float)state.Size.y * state.Scale * state.SpriteDepthFactor);
            m_pGhostTransform->LocalTransform.AddPosition(m_pGhostTransform->LocalTransform.GetRight() * (float)state.Size.x * state.Scale * state.SpriteLateralFactor);
        }
    }

    void Editor::UpdateGhostTint(const GhostState& state) const
    {
        int gridX = static_cast<int>(m_SelectorGridPos.x);
        int gridY = static_cast<int>(m_SelectorGridPos.y);
        glm::ivec2 pos(gridX, gridY);

        bool isFree = m_pMap->IsSpaceFree(pos, state.Size);

        m_pRender3D->RenderMaterial.EmissiveStrength = 0.0f;

        if (!state.HasStock)
            m_pRender3D->RenderMaterial.Tint = Colors::AMETHYST;
        else if (isFree)
            m_pRender3D->RenderMaterial.Tint = Colors::FOREST_GREEN;
        else
            m_pRender3D->RenderMaterial.Tint = Colors::DARK_RED;
    }

    void Editor::UpdateGhostVisuals()
    {
        GhostState state = GetCurrentGhostState();
        UpdateGhostGeometry(state);
        UpdateGhostPosition(state);
        UpdateGhostTint(state);
    }

    void Editor::PlaceBuilding()
    {
        glm::ivec2 pos = GetCursorGridPosition();
        BuildingType type = static_cast<BuildingType>(m_CurrentTypeIdx);
        glm::ivec2 size = m_pVillageManager->GetBuildingTemplate(type).Size;

        if (m_pMap->IsSpaceFree(pos, size))
        {
            if (m_pVillageManager->GetStashedCount(type) > 0)
            {
                m_BuildingCount--;
                m_pVillageManager->UnstashBuilding(type, pos);
                if (m_pVillageManager->GetStashedCount(type) == 0)
                    CycleToNextAvailable();
                m_LastTypeIdx = BuildingType::Count;
            }
        }
    }

    void Editor::StoreBuilding()
    {
        glm::ivec2 cursorPos = GetCursorGridPosition();

        for (const BuildingData& building : m_pVillageManager->GetBuildingsData())
        {
            if (building.IsStashed) continue;
            if (building.LinkedBehavior->Building_->Type == BuildingType::LifeTree) continue; // TODO C'est hyper sale mettre un truc pour auto
            if (IsCursorHoveringBuilding(cursorPos, building))
            {
                m_BuildingCount++;
                m_pVillageManager->StashBuildingAt(building.Position);
                CycleToNextAvailable();
                m_LastTypeIdx = BuildingType::Count;
                return;
            }
        }
    }
}
