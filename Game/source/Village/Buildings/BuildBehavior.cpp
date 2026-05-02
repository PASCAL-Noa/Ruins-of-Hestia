#include "UI/UiBuilder.h"
#include "Village/VillageManager.h"
#include "Village/Buildings/BuildingBehavior.h"

namespace GPC
{
    void BuildingBehavior::OnInteract()
    {
        IsInteracting = true;
        Render3DComponent_->SetTexture("1220x1683", Building_->TextureHighlightPath);
        VillageManager_->GetAudioSources()[1]->Play();
        Focus( { 6, -4} );
    }

    void BuildingBehavior::Focus(glm::vec2 offset)
    {
        auto camera =  VillageManager_->GetVillageController()->GetCamera();
        auto focusPoint = camera->pTarget;
        auto point = LocalScene->GetComponent<Transform3D>(GetEntityID())->GetWorldPosition();
        point.x += offset.x;
        point.z += offset.y;
        focusPoint->LocalTransform.SetPosition(point);
        camera->LockZoom = true;
        camera->Zoom = 10;
    }

    void BuildingBehavior::CloseOpened()
    {
        IsInteracting = false;
        VillageManager_->GetVillageController()->GetCamera()->LockZoom = false;
        const std::string& base = CurrentBasePath.empty() ? Building_->TexturePath : CurrentBasePath;
        Render3DComponent_->SetTexture("1220x1683", base);
    }

    void BuildingBehavior::OnCreate(const BehaviorCreateContext *pCtx)
    {
        Behavior::OnCreate(pCtx);
        LocalScene = pCtx->pScene;
    }

    void BuildingBehavior::OnUpdate(const BehaviorUpdateContext *pCtx)
    {
        Behavior::OnUpdate(pCtx);
    }

    void BuildingBehavior::CreateBaseInteractionUI(const BehaviorCreateContext *pCtx, UiCanvas** canvas)
    {
        UiBuilder b(pCtx->pScene);

        // --- Panel racine ---
        *canvas = b
            .At(Anchors::MIDDLE_LEFT, glm::vec2(0.0f, 0.0f))
            .Size({ 786.0f, 860.0f })
            .Tint(Colors::TRANSPARENT)
            .BuildCanvas();

        // --- Background sprite ---
        b.ChildOf((*canvas)->GetEntityID())
            .At(Anchors::MIDDLE_LEFT, glm::vec2(-50.0f, 0.0f))
            .Scale({ 0.75f, 0.75f })
            .Sprite("786x860", "Background_Upgrade")
            .BuildSprite();

        // --- Sub-canvas (contenu) ---
        UiCanvas* subCanvas = b.ChildOf((*canvas)->GetEntityID())
            .At(Anchors::MIDDLE_LEFT, glm::vec2(0.0f, -20.0f))
            .Size({ 600.0f, 400.0f })
            .Tint(Colors::TRANSPARENT)
            .BuildCanvas();

        // --- Nom du bâtiment ---
        b.ChildOf(subCanvas->GetEntityID())
            .At(Anchors::TOP_RIGHT, glm::vec2(-140.0f, 30.0f))
            .Font(UiTheme::DefaultFont)
            .Tint(UiTheme::DarkText)
            .FontSize(38)
            .Text(Building_->Name)
            .BuildText();

        if (Building_->DependentTo.Amount != 0) {
            // --- Icône de production ---
            b.ChildOf(subCanvas->GetEntityID())
                .At(Anchors::TOP_LEFT, glm::vec2(-20.0f, -15.0f))
                .Scale(0.8f)
                .Sprite("218x169", Building_->DependentTo.Texture)
                .BuildSprite();

            // --- Quantité produite ---
            b.ChildOf(subCanvas->GetEntityID())
                .At(Anchors::TOP_LEFT, glm::vec2(110.0f, 38.0f))
                .Font(UiTheme::DefaultFont)
                .Tint(UiTheme::DarkText)
                .FontSize(38)
                .Text(std::to_string(Building_->DependentTo.Amount))
                .BuildText();

            // --- Label "par cycle" ---
            b.ChildOf(subCanvas->GetEntityID())
                .At(Anchors::TOP_LEFT, glm::vec2(110.0f, 75.0f))
                .Font(UiTheme::DefaultFont)
                .Tint(UiTheme::DarkText)
                .FontSize(18)
                .Text("PAR CYCLE")
                .BuildText();
        }

        // Texte ameliorer
        b.ChildOf(subCanvas->GetEntityID())
            .At(Anchors::MIDDLE_MIDDLE, glm::vec2(-60.0f, -70.0f))
            .Font(UiTheme::DefaultFont)
            .Tint(UiTheme::DarkText)
            .FontSize(24)
            .Text("AMELIORER")
            .BuildText();

        // Panel
        UiSprite* upgrade = b.ChildOf(subCanvas->GetEntityID())
            .At(Anchors::MIDDLE_MIDDLE, glm::vec2(-75.0f, -60.0f))
            .Offset({ -60, -40})
            .Size({ 0.0f, 50.0f })
            .Scale({0.75f, 0.75f})
            .Sprite("580x157", "Button_Upgrade_Farm")
            .BuildSprite();

        UiButton* buttonUpgrade = b.BuildButton(upgrade);
        buttonUpgrade->OnClick.ConnectLambda([&] {
            // TODO Message ou upgrade
        });


        // --- Row upgrades (flex horizontal, centré) ---
        UiCanvas* upgradeRow = b.ChildOf(subCanvas->GetEntityID())
            .At(Anchors::MIDDLE_MIDDLE, glm::vec2(-25.0f, -25.0f))
            .Size({ 250.0f, 70.0f })
            .Tint(Colors::TRANSPARENT)
            .Layout(UiLayoutMode::Row)
            .Gap(0.0f)
            .Padding(0.0f)
            .Bg(Colors::TRANSPARENT)
            .Tint(Colors::TRANSPARENT)
            .BuildCanvas();

        for (const auto& upgrade : Building_->Upgrades)
        {
            UiCanvas* subPanelUpgrade = b.ChildOf(upgradeRow->GetEntityID())
                .FlexGrow(1.0f)
                .At(Anchors::TOP_LEFT, glm::vec2(0.0f, 0.0f))
                .Bg(Colors::TRANSPARENT)
                .Tint(Colors::TRANSPARENT)
                .BuildCanvas();

            // Montant upgrade
            b.ChildOf(subPanelUpgrade->GetEntityID())
                .Offset({ 0, 55})
                .Font(UiTheme::DefaultFont)
                .Tint(UiTheme::DarkText)
                .FontSize(26)
                .Text(std::to_string(upgrade.Amount))
                .BuildText();

            // Icône upgrade
            b.ChildOf(subPanelUpgrade->GetEntityID())
                .Offset({ -45, -20})
                .Size({ 0.0f, 50.0f })
                .Scale({0.5f, 0.5f})
                .Sprite("218x169", upgrade.Texture)
                .BuildSprite();

        }

        UiSprite* background = b.ChildOf(subCanvas->GetEntityID())
            .At(Anchors::MIDDLE_MIDDLE, { -60, 90})
            .Scale({0.75f, 0.75f})
            .Sprite("786x860", "Background_Farm_Text")
            .BuildSprite();

        b.ChildOf(background->GetEntityID())
            .At(Anchors::MIDDLE_MIDDLE, glm::vec2(0.0f, 0.0f))
            .Font(UiTheme::DefaultFont)
            .Tint(UiTheme::DarkText)
            .FontSize(24)
            .Text(Building_->Description)
            .BuildText();

    }
}