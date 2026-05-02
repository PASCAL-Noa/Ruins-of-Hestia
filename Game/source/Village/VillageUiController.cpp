#include "Village/VillageUiController.h"

#include "Scene.h"
#include "UI/UiBuilder.h"
#include "UI/UiButton.h"
#include "UI/UiCanvas.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"
#include "UI/UiTheme.h"
#include "Village/BuildingManager.h"
#include "Village/Inventory.h"
#include "Village/VillageManager.h"
#include "UiConstants.h"
#include "SettingsGame.h"
#include "UI/UiAnim.h"
#include "UI/UiAnimFluent.h"
#include "Village/Buildings/Barrack/Barrack.h"
#include "Village/Buildings/Farm/Farm.h"
#include "Village/Buildings/Forge/Forge.h"
#include "Village/Buildings/Hospital/Hospital.h"
#include "Village/Buildings/House/House.h"

namespace GPC
{
    VillageUiController::VillageUiController(Scene* pScene, VillageManager* pVillage, BuildingManager* pBuilding, UISettings* pSettings)
        : mp_Scene(pScene)
        , mp_Village(pVillage)
        , mp_Building(pBuilding)
        , mp_Settings(pSettings)
    {
    }

    void VillageUiController::Build()
    {
        if (!mp_Scene) return;

        UiBuilder b(mp_Scene);

        mp_Main = b.At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                   .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
                   .Tint(Colors::TRANSPARENT)
                   .FitToScreen(true)
                   .BuildCanvas();
        if (!mp_Main) return;

        BuildResourcesPanel(mp_Main->GetEntityID());
        BuildSettingsToggleButton(mp_Main->GetEntityID());
        BuildBuildingsPanel(mp_Main->GetEntityID());
        BuildBuildingsToggleButton(mp_Main->GetEntityID());
        BuildTooltipPanel(mp_Main->GetEntityID());
        BuildExpeditionButton(mp_Main->GetEntityID());
        BuildEditorButton(mp_Main->GetEntityID());


        ApplyVisibility();
    }

    void VillageUiController::Update()
    {
        if (!mp_Village) return;

        const auto& inv = mp_Village->GetInventory();
        m_Wood.Set(static_cast<int>(inv.GetResourceCount<Wood<ResourceTier::T1>>()));
        m_Stone.Set(static_cast<int>(inv.GetResourceCount<Stone<ResourceTier::T1>>()));
        m_Metal.Set(static_cast<int>(inv.GetResourceCount<Metal<ResourceTier::T1>>()));
        m_Fiber.Set(static_cast<int>(inv.GetResourceCount<Fiber<ResourceTier::T1>>()));
        m_Food.Set(static_cast<int>(inv.GetResourceCount<Food>()));
    }

    void VillageUiController::SetState(VillageUiState state)
    {
        m_State = state;
        ApplyVisibility();
    }

    void VillageUiController::OpenTooltip(const std::string& title, const std::string& desc)
    {
        m_TooltipTitle.Set(title);
        m_TooltipDesc.Set(desc);
        SetState(VillageUiState::Tooltip);
    }

    void VillageUiController::EnterDialogue()
    {
        SetState(VillageUiState::Dialogue);
    }

    void VillageUiController::LeaveDialogue()
    {
        if (m_State == VillageUiState::Dialogue)
        {
            SetState(VillageUiState::Idle);
        }
    }

    UiCanvas* VillageUiController::GetMainCanvas() const
    {
        return mp_Main;
    }

    UiSprite* VillageUiController::GetBuildingButton() const
    {
        return mp_BuildingButton;
    }

    UiSprite* VillageUiController::GetExpeditionButton() const
    {
        return mp_ExpeditionButton;
    }

    UiSprite* VillageUiController::GetEditorButton() const
    {
        return mp_EditorButton;
    }

    UiSprite* VillageUiController::GetSettingsButton() const
    {
        return mp_SettingsButton;
    }

    void VillageUiController::ApplyVisibility()
    {
        const bool showMain      = (m_State != VillageUiState::Dialogue);
        const bool showSettings  = (m_State == VillageUiState::Settings);
        const bool showBuildings = (m_State == VillageUiState::Buildings) || (m_State == VillageUiState::Tooltip);
        const bool showTooltip   = (m_State == VillageUiState::Tooltip);

        if (mp_Main)      mp_Main->IsEnable = showMain;
        if (mp_Settings)  mp_Settings->Toggle(showSettings);
        if (mp_Buildings) mp_Buildings->IsEnable = showBuildings;
        if (mp_Tooltip)   mp_Tooltip->IsEnable = showTooltip;

    }

    void VillageUiController::BuildResourcesPanel(EntityID parent)
    {
        UiBuilder b(mp_Scene);

        UiSprite* bg = b.ChildOf(parent)
                        .At(Anchors::TOP_LEFT, { 0.0f, -0.5f })
                        .Size({ 529.0f, 106.0f })
                        .Sprite("529x106", "Resources_Background")
                        .BuildSprite();
        if (!bg) return;

        UiAnim::On(*mp_Scene, bg)
               .FadeTo(1.0f, 2.5f, Tweening::EasingType::Linear)
               .OnUpdate([bg](float t) {
                       int frameNum = static_cast<int>(t * 14.9f);
                       std::string frameName = "ANIM";
                       if (frameNum < 10) frameName += "0";
                       frameName += std::to_string(frameNum) + "_Resources_Background";

                       bg->SetSprite("ANIMATION_Resources_Background", frameName);
               })
               .Repeat(-1)
               .Play();

        const EntityID bgId = bg->GetEntityID();

        float currentX = 60.0f;
        const float pos_icon_y = -15.0f;
        const float gap = 75.0f;
        const float margin_right = -60.0f;
        const glm::vec2 iconSize = { 123.0f, 95.0f };

        const glm::vec2 iconScale = { iconSize.x / 218.0f, iconSize.y / 169.0f };

        auto makeSlot = [&](const std::string& iconName, UiReactive<int>& source)
        {
            UiSprite* icon = b.ChildOf(bgId)
                              .At(Anchors::MIDDLE_LEFT, { currentX + margin_right, pos_icon_y })
                              .Sprite("218x169", iconName)
                              .Scale(iconScale)
                              .BuildSprite();
            if (!icon) return;

            UiText* valueLabel = b.ChildOf(icon->GetEntityID())
                                   .At(Anchors::BOTTOM_MIDDLE, { 0.0f, -10.0f })
                                   .FontSize(18.0f)
                                   .Tint(Colors::PERFECT_BROWN)
                                   .Text("0")
                                   .BuildText();
            if (valueLabel) valueLabel->BindReactive(source);

            currentX += gap;
        };

        makeSlot("Productivity_Wood",  m_Wood);
        makeSlot("Productivity_Stone", m_Stone);
        makeSlot("Productivity_Iron",  m_Metal);
        makeSlot("Productivity_Fiber", m_Fiber);
        makeSlot("Productivity_Food",  m_Food);
    }

    void VillageUiController::BuildSettingsToggleButton(EntityID parent)
    {
        UiBuilder b(mp_Scene);

        mp_SettingsButton = b.ChildOf(parent)
                         .At(Anchors::TOP_RIGHT, { 0.0f, 0.0f })
                         .Sprite("218x169", "Button_Settings")
                         .BuildSprite();
        if (!mp_SettingsButton) return;

        UiButton* button = b.BuildButton(mp_SettingsButton);
        if (!button) return;
        button->OnClick.ConnectLambda([this]()
        {
            SetState(m_State == VillageUiState::Settings ? VillageUiState::Idle : VillageUiState::Settings);
        });
    }

    void VillageUiController::BuildBuildingRowUI(UiBuilder &b, UiCanvas* parentPanel,
        const std::string &iconStr, const std::string &title, const std::string &description,
        Callable<void> *buildAction)
    {
        // --- Row container (bouton de fond) ---
        UiCanvas* row = b.ChildOf(parentPanel->GetEntityID())
            .At(Anchors::TOP_LEFT, glm::vec2(0.0f, 0.0f))
            .Size({ 580.0f, 55.0f })
            .Bg(UiTheme::ButtonPrimary)
            .BuildCanvas();

        UiButton* action = b.BuildButton(row);
        action->OnClick.ConnectLambda([buildAction]()
        {
            if (buildAction) buildAction->Call();
        });

        // --- Icône du bâtiment ---
        b.ChildOf(row->GetEntityID())
            .At(Anchors::MIDDLE_LEFT, glm::vec2(8.0f, 0.0f))
            .Scale({ 0.3f, 0.3f })
            .Sprite("218x169", iconStr)
            .BuildSprite();

        // --- Titre ---
        b.ChildOf(row->GetEntityID())
            .At(Anchors::MIDDLE_LEFT, glm::vec2(90.0f, 0.0f))
            .Font(UiTheme::DefaultFont)
            .Tint(Colors::PERFECT_BROWN)
            .FontSize(UiTheme::BodyFontSize)
            .Text(title)
            .BuildText();

        // --- Bouton tooltip ---
        UiSprite* tooltipBtn = b.ChildOf(row->GetEntityID())
            .At(Anchors::MIDDLE_RIGHT, glm::vec2(-8.0f, 0.0f))
            .Scale({ 0.7f, 0.7f })
            .Sprite("218x169", "Button_Tooltip")
            .BuildSprite();

        UiButton* tooltipAction = b.BuildButton(tooltipBtn);
        tooltipAction->OnClick.ConnectLambda([this, title, description]()
        {
            if (!mp_Buildings->IsEnable || !mp_Buildings->IsEnable) return;
            OpenTooltip(title, description);
        });
    }

    void VillageUiController::BuildBuildingsToggleButton(EntityID parent)
    {
        UiBuilder b(mp_Scene);

        mp_BuildingButton = b.ChildOf(parent)
                         .At(Anchors::BOTTOM_LEFT, { 4.0f, -72.5f })
                         .Sprite("218x169", "Button_Buildings")
                         .Scale({ 0.6f, 0.6f })
                         .BuildSprite();
        if (!mp_BuildingButton) return;



        UiButton* button = b.BuildButton(mp_BuildingButton);
        if (!button) return;
        button->OnClick.ConnectLambda([this]()
        {
            const bool open = (m_State != VillageUiState::Buildings && m_State != VillageUiState::Tooltip);
            SetState(open ? VillageUiState::Buildings : VillageUiState::Idle);
        });
    }

    void VillageUiController::BuildBuildingsPanel(EntityID parent)
    {
        UiBuilder b(mp_Scene);

        mp_Buildings = b.ChildOf(parent)
                       .At(Anchors::MIDDLE_LEFT, { 0.0f, 0.0f })
                       .Size(kBuildingPanelSize)
                       .Tint(Colors::TRANSPARENT)
                       .BuildCanvas();
        if (!mp_Buildings) return;

        UiSprite* SpriteBuilding = b.ChildOf(mp_Buildings->GetEntityID())
                                     .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                                     .Scale({ 0.6f, 0.6f })
                                     .Sprite("786x860", "Background_Buildings")
                                     .BuildSprite();
        if (!SpriteBuilding) return;

        UiAnim::On(*mp_Scene, SpriteBuilding)
               .FadeTo(1.0f, 2.5f, Tweening::EasingType::Linear)
               .OnUpdate([SpriteBuilding](float t) {
                   if (SpriteBuilding->IsEnable)
                   {
                       int frameNum = static_cast<int>(t * 15.0f);
                       if (frameNum >= 15) frameNum = 0;
                       std::string frameName = "ANIM";
                       if (frameNum < 10) frameName += "0";
                       frameName += std::to_string(frameNum) + "_Building";

                       SpriteBuilding->SetSprite("ANIMATION_Building", frameName);
                   }
               })
               .Repeat(-1)
               .Play();

        mp_BuildingResourcesSprite.resize(static_cast<uint32_t>(BuildingType::Count));
        AddBuildingRow<House>(mp_Buildings->GetEntityID(), 0);
        AddBuildingRow<Farm>(mp_Buildings->GetEntityID(), 1);
        AddBuildingRow<Forge>(mp_Buildings->GetEntityID(), 2);
        AddBuildingRow<Hospital>(mp_Buildings->GetEntityID(), 3);
        AddBuildingRow<Barrack>(mp_Buildings->GetEntityID(), 4);

    }

    void VillageUiController::BuildTooltipPanel(EntityID parent)
    {
        UiBuilder b(mp_Scene);

        mp_Tooltip = b.ChildOf(parent)
                     .At(Anchors::MIDDLE_LEFT, { 285.0f, 158.0f })
                     .Size(kTooltipPanelSize)
                     .Tint(Colors::TRANSPARENT)
                     .BuildCanvas();
        if (!mp_Tooltip) return;

        UiSprite* SpriteToolTips = b.ChildOf(mp_Tooltip->GetEntityID())
                                     .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                                     .Scale({ 0.6f, 0.6f })
                                     .Sprite("786x860", "Background_Tooltip")
                                     .BuildSprite();
        if (!SpriteToolTips) return;

        UiAnim::On(*mp_Scene, SpriteToolTips)
               .FadeTo(1.0f, 2.5f, Tweening::EasingType::Linear)
               .OnUpdate([SpriteToolTips](float t) {
                   if (SpriteToolTips->IsEnable)
                   {
                       int frameNum = static_cast<int>(t * 15.0f);
                       if (frameNum >= 15) frameNum = 0;
                       std::string frameName = "ANIM";
                       if (frameNum < 10) frameName += "0";
                       frameName += std::to_string(frameNum) + "_ToolTips";

                       SpriteToolTips->SetSprite("ANIMATION_ToolTips", frameName);
                   }
               })
               .Repeat(-1)
               .Play();

        UiText* titleLabel = b.ChildOf(mp_Tooltip->GetEntityID())
                               .At(Anchors::TOP_MIDDLE, { -145.0f, 120.0f })
                               .FontSize(24.0f)
                               .Tint(Colors::PERFECT_BROWN)
                               .Text("")
                               .BuildText();
        if (titleLabel) titleLabel->BindReactive(m_TooltipTitle);

        UiText* descLabel = b.ChildOf(mp_Tooltip->GetEntityID())
                              .At(Anchors::TOP_MIDDLE, { -200.0f, 160.0f })
                              .FontSize(16.0f)
                              .Tint(Colors::PERFECT_BROWN)
                              .Text("")
                              .BuildText();
        if (descLabel) descLabel->BindReactive(m_TooltipDesc);
    }

    void VillageUiController::BuildExpeditionButton(EntityID parent)
    {
        UiBuilder b(mp_Scene);

        mp_ExpeditionButton = b.ChildOf(parent)
                         .At(Anchors::BOTTOM_RIGHT, { -8.0f, -72.5f })
                         .Sprite("218x169", "Button_Expedition")
                         .Scale({ 0.6f, 0.6f })
                         .BuildSprite();
        if (!mp_ExpeditionButton) return;

        UiButton* button = b.BuildButton(mp_ExpeditionButton);
        if (!button) return;
        button->OnClick.ConnectLambda([this]()
        {
            OnExpeditionRequested.Emit();
        });
    }

    void VillageUiController::BuildEditorButton(EntityID parent)
    {
        UiBuilder b(mp_Scene);

        mp_EditorButton = b.ChildOf(mp_Main->GetEntityID())
            .At(Anchors::BOTTOM_RIGHT, { 8, -165 })
            .Scale( {0.5f, 0.5f} )
            .Sprite("250x180", "EditMode_Icon")
            .BuildSprite();


        if (!mp_EditorButton) return;

        UiButton* button = b.BuildButton(mp_EditorButton);
        if (!button) return;
        button->OnClick.ConnectLambda([this]()
        {
            mp_Village->GetVillageController()->StartEditing();
        });

    }
}
