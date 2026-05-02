#pragma once

#include <string>

#include "Building.h"
#include "ECS_Defines.h"
#include "UiConstants.h"
#include "UI/UiBuilder.h"
#include "Village/BuildingData.h"
#include "UI/UiReactive.h"
#include "UI/UiSignal.h"
#include "UI/UiBuilder.h"
#include "UISettingsGame_Behavior.h"
#include "VillageManager.h"
#include "UI/UiAnim.h"
#include "UI/UiAnimFluent.h"

namespace GPC
{
    struct Building;
    struct Resource;
    class Scene;
    class VillageManager;
    class BuildingManager;
    struct UiCanvas;
    struct UiText;
    struct UiSprite;

    enum class VillageUiState : uint8_t
    {
        Idle      = 0,
        Settings  = 1,
        Buildings = 2,
        Tooltip   = 3,
        Dialogue  = 4
    };

    class VillageUiController
    {
    public:
        VillageUiController(Scene* pScene, VillageManager* pVillage, BuildingManager* pBuilding, UISettings* pSettings);

        void                    Build();
        void                    Update();

        void                    SetState(VillageUiState state);
        VillageUiState          GetState() const { return m_State; }

        void                    OpenTooltip(const std::string& title, const std::string& desc);
        void                    EnterDialogue();
        void                    LeaveDialogue();

        UiCanvas*               GetMainCanvas() const;

        UiSprite*               GetBuildingButton() const;
        UiSprite*               GetExpeditionButton() const;
        UiSprite*               GetEditorButton() const;
        UiSprite*               GetSettingsButton() const;

        UiSignal<>              OnExpeditionRequested;
        UiSignal<BuildingType>  OnBuildingRequested;

    private:
        void                    BuildBuildingsPanel(EntityID parent);
        void                    BuildTooltipPanel(EntityID parent);
        void                    BuildResourcesPanel(EntityID parent);
        void                    BuildExpeditionButton(EntityID parent);
        void                    BuildEditorButton(EntityID parent);
        void                    BuildBuildingsToggleButton(EntityID parent);
        void                    BuildSettingsToggleButton(EntityID parent);
        void                    BuildBuildingRowUI(UiBuilder &b, UiCanvas* parentPanel,
        const std::string &iconStr, const std::string &title, const std::string &description,
        Callable<void> *buildAction);

        template<typename Building>
        void                    AddBuildingRow(EntityID parent, int index);

        void                    ApplyVisibility();

    private:
        Scene*                  mp_Scene    = nullptr;
        VillageManager*         mp_Village  = nullptr;
        BuildingManager*        mp_Building = nullptr;
        UISettings*             mp_Settings = nullptr;

        VillageUiState          m_State     = VillageUiState::Idle;

        UiCanvas*               mp_Main      = nullptr;
        UiCanvas*               mp_Buildings = nullptr;
        UiCanvas*               mp_Tooltip   = nullptr;
        std::vector<std::vector<UiSprite*>> mp_BuildingResourcesSprite = {};

        UiReactive<int>         m_Wood{ 0 };
        UiReactive<int>         m_Stone{ 0 };
        UiReactive<int>         m_Metal{ 0 };
        UiReactive<int>         m_Fiber{ 0 };
        UiReactive<int>         m_Food{ 0 };

        UiReactive<std::string> m_TooltipTitle{ "" };
        UiReactive<std::string> m_TooltipDesc{ "" };

        UiSprite*               mp_BuildingButton = nullptr;
        UiSprite*               mp_ExpeditionButton = nullptr;
        UiSprite*               mp_EditorButton = nullptr;
        UiSprite*               mp_SettingsButton = nullptr;
    };

    template<typename Building>
    void VillageUiController::AddBuildingRow(EntityID parent, int index)
    {

        Building build;

        UiBuilder b(mp_Scene);
        const float currentY = kBuildingRowStartY + static_cast<float>(index) * kBuildingRowHeight;

        UiSprite* buttonBg = b.ChildOf(parent)
                               .At(Anchors::TOP_LEFT, { 20.0f, currentY - 25.0f })
                               .Scale({ 0.5f, 0.42f })
                               .Sprite("580x157", "Button_Upgrade")
                               .BuildSprite();
        if (!buttonBg) return;

        UiButton* buildBtn = b.BuildButton(buttonBg);
        if (buildBtn)
        {
            buildBtn->OnClick.ConnectLambda([&, index, build]()
            {
                if (!mp_Village->CanAfford(build.Type)) {
                    for (auto button : mp_BuildingResourcesSprite[index]) {
                        UiAnim::ScalePulse(*mp_Scene, button, 0.33f, 0.2f, Tweening::EasingType::EaseInOutBack, 0.3f, 0.3f);
                    }
                    return;
                }
                if (m_State != VillageUiState::Buildings && m_State != VillageUiState::Tooltip) return;
                if (!build.Unlocked) return;
                OnBuildingRequested.Emit(build.Type);
                SetState(VillageUiState::Idle);
            });
        }

        b.ChildOf(parent)
         .At(Anchors::TOP_LEFT, { 30.0f, currentY - 23.0f })
         .Scale({ 0.3f, 0.3f })
         .Sprite("218x169", build.Icon)
         .BuildSprite();

        b.ChildOf(parent)
         .At(Anchors::TOP_MIDDLE, { -70.0f, currentY - 12.0f })
         .Font(UiTheme::DefaultFont, 24.0f)
         .Tint(UiTheme::DarkText)
         .Text(build.Name)
         .BuildText();

        // --- Row upgrades (flex horizontal, centré) ---

        uint32_t resourceIndex = 0;
        UiCanvas* upgradeRow = b.ChildOf(parent)
                    .At(Anchors::TOP_MIDDLE, glm::vec2(0, currentY - 45))
                    .Layout(UiLayoutMode::Row)
                    .Bg(Colors::ALICE_BLUE)
                    .Gap(0.0f)
                    .Padding(0.0f)
                    .BuildCanvas();

        for (BuildingNeeds resource : build.Upgrades) {

            UiCanvas* subPanelUpgrade = b.ChildOf(upgradeRow->GetEntityID())
                .FlexGrow(1.0f)
                .At(Anchors::TOP_LEFT, glm::vec2(0.0f, 0.0f))
                .BuildCanvas();

            mp_BuildingResourcesSprite[index].push_back(b.ChildOf(subPanelUpgrade->GetEntityID())
                 .At(Anchors::TOP_MIDDLE, { -110.0f + resourceIndex * 40, 40.0f })
                 .Scale({ 0.3f, 0.3f })
                 .Sprite("218x169", resource.Texture)
                 .BuildSprite());

            b.ChildOf(subPanelUpgrade->GetEntityID())
                 .At(Anchors::TOP_MIDDLE, { -95.0f + resourceIndex * 40, 52.0f })
                 .Font(UiTheme::DefaultFont, 12.0f)
                 .Tint(UiTheme::DarkText)
                 .Text(std::to_string(resource.Amount))
                 .BuildText();

            resourceIndex++;
        }

        UiSprite* tooltipBtn = b.ChildOf(parent)
                                 .At(Anchors::TOP_RIGHT, { -120.0f, currentY - 55.0f })
                                 .Scale({ 0.7f, 0.7f })
                                 .Sprite("218x169", "Button_Tooltip")
                                 .BuildSprite();
        if (tooltipBtn)
        {
            UiButton* tooltipButton = b.BuildButton(tooltipBtn);
            if (tooltipButton)
            {
                tooltipButton->OnClick.ConnectLambda([this, build]()
                {
                    if (m_State != VillageUiState::Buildings && m_State != VillageUiState::Tooltip) return;
                    OpenTooltip(build.Name, build.Description);
                });
            }
        }
    }
}
