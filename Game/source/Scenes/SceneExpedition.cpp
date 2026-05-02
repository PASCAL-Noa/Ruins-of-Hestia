#include "Scenes/SceneExpedition.h"

#include <cmath>
#include <filesystem>

#include "AnimationComponent.h"
#include "Assets.h"
#include "AudioSystem.h"
#include "FbxLoader.h"
#include "FollowTarget.h"
#include "FreeCamera.h"
#include "LightingSystem.h"
#include "Random.h"
#include "../../include/Expedition/PlayerController.h"
#include "TransformComponents.h"
#include "Render3DSystem.h"
#include "SettingsGame.h"
#include "Map Generation/TerrainGeneratorBehavior.h"
#include "Map Generation/MapGenerator.h"
#include "UI/UiBuilder.h"
#include "UI/UiCanvas.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"
#include "UI/UiTheme.h"
#include "UiConstants.h"
#include "Expedition/TutoBehaviors.h"
#include "TransiFeuillle.h"
#include "Expedition/IrisFollowing.h"
#include "UI/UiAnim.h"
#include "Scenes/SceneVillageV1.h"
#include  "Expedition/BossBarHandler.h"


namespace GPC
{
    void SceneExpedition::SetupPostGameUI()
    {
        UiBuilder b(this);
        const EntityID mainId = mp_MainUI->GetEntityID();

        mp_PostGameUI = b.ChildOf(mainId)
            .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
            .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
            .Tint(Color(0, 0, 0, 120))
            .FitToScreen(true)
            .BuildCanvas();
        if (!mp_PostGameUI) return;
        mp_PostGameUI->IsEnable = false;
        const EntityID overlayId = mp_PostGameUI->GetEntityID();

        mp_bgSprite = b.ChildOf(overlayId)
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
            .Scale({ 0.72f, 0.72f })
            .Sprite("ANIMATION_PostGame_BG", "ANIM00_PostGame_BG")
            .BuildSprite();
        if (!mp_bgSprite) return;
        const EntityID bgId = mp_bgSprite->GetEntityID();

        UiAnim::On(*this, mp_bgSprite)
            .FadeTo(1.0f, 2.0f, Tweening::EasingType::Linear)
            .OnUpdate([this](float t)
            {
                int frameNum = static_cast<int>(t * 14.9f);
                std::string frameName = "ANIM" + std::string(frameNum < 10 ? "0" : "") + std::to_string(frameNum) + "_PostGame_BG";
                mp_bgSprite->SetSprite("ANIMATION_PostGame_BG", frameName);
            })
            .Repeat(-1)
            .Play();

        // - Textes titre
        mp_PostGameZoneText = b.ChildOf(bgId)
            .At(Anchors::TOP_MIDDLE, { 0.0f, 55.0f })
            .Font(UiTheme::DefaultFont, 20)
            .Tint(Colors::PERFECT_BROWN)
            .Text("ZONE 1")
            .BuildText();

        mp_TitleText = b.ChildOf(bgId)
            .At(Anchors::TOP_MIDDLE, { 0.0f, 85.0f })
            .Font(UiTheme::DefaultFont, 56)
            .Tint(Colors::PERFECT_BROWN)
            .Text("ACCOMPLIE")
            .BuildText();

        b.ChildOf(bgId)
            .At(Anchors::TOP_MIDDLE, { 0.0f, 150.0f })
            .Font(UiTheme::DefaultFont, 20)
            .Tint(Colors::PERFECT_BROWN)
            .Text("RECOMPENSES")
            .BuildText();

        // - Rangée ressources de base
        struct IconDef { const char* texName; UiText** ppText; };
        IconDef baseIcons[] = {
            { "PostGame_Fiber", &mp_PostGameFiberText },
            { "PostGame_Wood",  &mp_PostGameWoodText  },
            { "PostGame_Food",  &mp_PostGameFoodText  },
            { "PostGame_Stone", &mp_PostGameStoneText },
            { "PostGame_Ore",   &mp_PostGameOreText   },
        };
        constexpr float BASE_START_X = -220.0f;
        constexpr float BASE_SPACING = 110.0f;
        constexpr float BASE_Y       = 180.0f;

        for (int i = 0; i < 5; ++i)
        {
            mp_PostGameBaseIcons[i] = b.ChildOf(bgId)
                .At(Anchors::TOP_MIDDLE, { BASE_START_X + i * BASE_SPACING, BASE_Y })
                .Scale({ 0.85f, 0.85f })
                .Sprite("95x93", baseIcons[i].texName)
                .BuildSprite();
            if (!mp_PostGameBaseIcons[i]) continue;

            *baseIcons[i].ppText = b.ChildOf(mp_PostGameBaseIcons[i]->GetEntityID())
                .At(Anchors::BOTTOM_MIDDLE, { 0.0f, 30.0f })
                .Font(UiTheme::DefaultFont, 24)
                .Tint(Colors::PERFECT_BROWN)
                .Text("0")
                .BuildText();
        }

        // - Rangée gemmes
        IconDef crystalIcons[] = {
            { "PostGame_Crystal_T1", &mp_PostGameCrystalT1Text },
            { "PostGame_Crystal_T2", &mp_PostGameCrystalT2Text },
            { "PostGame_Crystal_T3", &mp_PostGameCrystalT3Text },
            { "PostGame_Crystal_T4", &mp_PostGameCrystalT4Text },
            { "PostGame_Crystal_T5", &mp_PostGameCrystalT5Text },
            { "PostGame_Crystal_T6", &mp_PostGameCrystalT6Text },
        };
        constexpr float CRYSTAL_START_X = -275.0f;
        constexpr float CRYSTAL_SPACING =  110.0f;
        constexpr float CRYSTAL_Y       =  330.0f;

        for (int i = 0; i < 6; ++i)
        {
            mp_PostGameCrystalIcons[i] = b.ChildOf(bgId)
                .At(Anchors::TOP_MIDDLE, { CRYSTAL_START_X + i * CRYSTAL_SPACING, CRYSTAL_Y })
                .Scale({ 0.85f, 0.85f })
                .Sprite("95x93", crystalIcons[i].texName)
                .BuildSprite();
            if (!mp_PostGameCrystalIcons[i]) continue;

            *crystalIcons[i].ppText = b.ChildOf(mp_PostGameCrystalIcons[i]->GetEntityID())
                .At(Anchors::BOTTOM_MIDDLE, { 0.0f, 30.0f })
                .Font(UiTheme::DefaultFont, 24)
                .Tint(Colors::PERFECT_BROWN)
                .Text("0")
                .BuildText();
        }

        auto createPostGameButton = [&](
        float posX, float posY,
        const std::string& label,
        const std::string& normalName,
        const std::string& selectedName,
        Callable<void>* action) -> UiButton*
        {
            UiSprite* btnSprite = b.ChildOf(bgId)
                .At(Anchors::MIDDLE_MIDDLE, { (int)posX, (int)posY })
                .Scale({ 0.45f, 0.45f })
                .Sprite("PostGame_Buttons_Normal", normalName)
                .BuildSprite();
            if (!btnSprite) return nullptr;

            const EntityID btnID = btnSprite->GetEntityID();
            UiButton* btn = b.BuildButton(btnSprite);

            b.At(Anchors::MIDDLE_MIDDLE, { 0, 5 })
                .Font(UiTheme::DefaultFont)
                .FontSize(22)
                .Tint(Colors::PERFECT_BROWN)
                .Text(label)
                .ChildOf(btnID)
                .BuildText();

            btn->OnHover.ConnectLambda([btnSprite, selectedName]() {
                btnSprite->SetSprite("PostGame_Buttons_Selected", selectedName);
            });

            btn->OnLeave.ConnectLambda([btnSprite, normalName]() {
                btnSprite->SetSprite("PostGame_Buttons_Normal", normalName);
            });

            if (btn && action) btn->OnClick.Connect(action);
            return btn;
        };

        // ── Bouton "RETOUR AU VILLAGE"
        createPostGameButton(-220.0f, 220.0f, "RETOUR AU VILLAGE",
            "PostGame_Btn1_Normal", "PostGame_Btn1_Selected",
            MakeCallablePtr([this]()
            {
                mp_PostGameUI->IsEnable = false;
                if (mp_CharacterUi)            mp_CharacterUi->IsEnable            = true;
                if (mp_MaterialUi)             mp_MaterialUi->IsEnable             = true;
                if (mp_MiniMap)                mp_MiniMap->IsEnable                = true;
                if (mp_MiniMapBackground)      mp_MiniMapBackground->IsEnable      = true;
                if (mp_PlayerMiniMapIndicator) mp_PlayerMiniMapIndicator->IsEnable = true;
                if (mp_PlayerControllerBehavior) mp_PlayerControllerBehavior->SetEnabled(true);

                if (mp_PlayerLivingBehavior && mp_Inventory)
                {
                    auto pushBack = [&](Equipment* pEq)
                    {
                        if (pEq == nullptr) return;
                        pEq->EquipTo(nullptr);
                        mp_Inventory->AddEquipment(pEq);
                    };
                    pushBack(mp_PlayerLivingBehavior->GetHelmet());
                    pushBack(mp_PlayerLivingBehavior->GetChestPlate());
                    pushBack(mp_PlayerLivingBehavior->GetGloves());
                    pushBack(mp_PlayerLivingBehavior->GetLegs());
                    pushBack(mp_PlayerLivingBehavior->GetBoots());
                    pushBack(mp_PlayerLivingBehavior->GetMelee());
                    pushBack(mp_PlayerLivingBehavior->GetDistance());
                }

                SceneVillageStartCtx* village_ctx = new SceneVillageStartCtx();
                village_ctx->pExpeditionInventory = mp_Inventory;
                SceneChangeCtx ctx{};
                ctx.pData = village_ctx;
                ctx.DataSize = sizeof(SceneVillageStartCtx);
                if (mp_PlayerControllerBehavior) mp_PlayerControllerBehavior->SetEnabled(true);
                if (mp_EnnemiGeneratorBehavior)  mp_EnnemiGeneratorBehavior->SetEnabled(true);
                if (mp_PlayerLivingBehavior)     mp_PlayerLivingBehavior->SetEnabled(true);
                if (mp_PlayerInteractionBehavior) mp_PlayerInteractionBehavior->SetEnabled(true);
                if (mp_IrisBehavior)              mp_IrisBehavior->SetEnabled(true);
                if (mp_BackgroundAdminPanel)  mp_BackgroundAdminPanel->IsEnable = true;
                if (mp_BackgroundAdminPanel)  mp_BackgroundAdminPanel->IsEnable = true;
                SwapToScene("Village", ctx);
            })
        );

        mp_ContinueButtonText = b.ChildOf(bgId)
            .At(Anchors::MIDDLE_MIDDLE, { 220.0f, 180.0f })
            .Font(UiTheme::DefaultFont, 20)
            .Tint(Colors::DARK_RED)
            .Text("Si vous mourrez, vous risquez gros.")
            .BuildText();

        // ── Bouton "CONTINUER"
        mp_ContinueButton = createPostGameButton(220.0f, 220.0f, "CONTINUER",
            "PostGame_Btn2_Normal", "PostGame_Btn2_Selected",
            MakeCallablePtr([this]()
            {
                mp_PostGameUI->IsEnable = false;
                if (mp_CharacterUi)            mp_CharacterUi->IsEnable            = true;
                if (mp_MaterialUi)             mp_MaterialUi->IsEnable             = true;
                if (mp_MiniMap)                mp_MiniMap->IsEnable                = true;
                if (mp_MiniMapBackground)      mp_MiniMapBackground->IsEnable      = true;
                if (mp_PlayerMiniMapIndicator) mp_PlayerMiniMapIndicator->IsEnable = true;
                if (mp_PlayerControllerBehavior) mp_PlayerControllerBehavior->SetEnabled(true);
                SceneExpeditionStartCtx* nextCtx = new SceneExpeditionStartCtx();
                nextCtx->MissionDifficulty = m_Difficulty + 1;
                if (mp_PlayerLivingBehavior)
                {
                    nextCtx->pHelmet     = mp_PlayerLivingBehavior->GetHelmet();
                    nextCtx->pGloves     = mp_PlayerLivingBehavior->GetGloves();
                    nextCtx->pChestPlate = mp_PlayerLivingBehavior->GetChestPlate();
                    nextCtx->pLegs       = mp_PlayerLivingBehavior->GetLegs();
                    nextCtx->pBoots      = mp_PlayerLivingBehavior->GetBoots();
                    nextCtx->pMelee      = mp_PlayerLivingBehavior->GetMelee();
                    nextCtx->pDistance   = mp_PlayerLivingBehavior->GetDistance();
                }
                SceneChangeCtx ctx{};
                ctx.pData = nextCtx;
                ctx.DataSize = sizeof(SceneExpeditionStartCtx);
                if (mp_PlayerControllerBehavior) mp_PlayerControllerBehavior->SetEnabled(true);
                if (mp_EnnemiGeneratorBehavior)  mp_EnnemiGeneratorBehavior->SetEnabled(true);
                if (mp_PlayerLivingBehavior)     mp_PlayerLivingBehavior->SetEnabled(true);
                if (mp_PlayerInteractionBehavior) mp_PlayerInteractionBehavior->SetEnabled(true);
                if (mp_IrisBehavior)              mp_IrisBehavior->SetEnabled(true);
                if (mp_BackgroundAdminPanel)  mp_BackgroundAdminPanel->IsEnable = true;
                if (mp_BackgroundAdminPanel)  mp_BackgroundAdminPanel->IsEnable = true;
                SwapToScene("Expedition", ctx);
            })
        );
    }

    void SceneExpedition::ShowPostGameUI(uint32_t zone, bool canContinue)
    {
        if (!mp_PostGameUI) return;
        mp_WinSFX->Play();

        if (mp_PostGameZoneText)
            mp_PostGameZoneText->SetText("ZONE " + std::to_string(zone));

        if (mp_Inventory)
        {
            auto sumTiers = [&](uint32_t t1, uint32_t t2, uint32_t t3, uint32_t t4, uint32_t t5) {
                return t1 + t2 + t3 + t4 + t5;
            };

            uint32_t counts[5] = {
                sumTiers(
                    mp_Inventory->GetResourceCount<Fiber<ResourceTier::T1>>(),
                    mp_Inventory->GetResourceCount<Fiber<ResourceTier::T2>>(),
                    mp_Inventory->GetResourceCount<Fiber<ResourceTier::T3>>(),
                    mp_Inventory->GetResourceCount<Fiber<ResourceTier::T4>>(),
                    mp_Inventory->GetResourceCount<Fiber<ResourceTier::T5>>()),
                sumTiers(
                    mp_Inventory->GetResourceCount<Wood<ResourceTier::T1>>(),
                    mp_Inventory->GetResourceCount<Wood<ResourceTier::T2>>(),
                    mp_Inventory->GetResourceCount<Wood<ResourceTier::T3>>(),
                    mp_Inventory->GetResourceCount<Wood<ResourceTier::T4>>(),
                    mp_Inventory->GetResourceCount<Wood<ResourceTier::T5>>()),
                mp_Inventory->GetResourceCount<Food>(),
                sumTiers(
                    mp_Inventory->GetResourceCount<Stone<ResourceTier::T1>>(),
                    mp_Inventory->GetResourceCount<Stone<ResourceTier::T2>>(),
                    mp_Inventory->GetResourceCount<Stone<ResourceTier::T3>>(),
                    mp_Inventory->GetResourceCount<Stone<ResourceTier::T4>>(),
                    mp_Inventory->GetResourceCount<Stone<ResourceTier::T5>>()),
                sumTiers(
                    mp_Inventory->GetResourceCount<Metal<ResourceTier::T1>>(),
                    mp_Inventory->GetResourceCount<Metal<ResourceTier::T2>>(),
                    mp_Inventory->GetResourceCount<Metal<ResourceTier::T3>>(),
                    mp_Inventory->GetResourceCount<Metal<ResourceTier::T4>>(),
                    mp_Inventory->GetResourceCount<Metal<ResourceTier::T5>>()),
            };

            UiText* baseTexts[5] = {
                mp_PostGameFiberText, mp_PostGameWoodText, mp_PostGameFoodText,
                mp_PostGameStoneText, mp_PostGameOreText
            };

            //Repositionnement dynamique des icônes de base
            {
                constexpr float SPACING = 110.0f;
                constexpr float BASE_Y  = 180.0f;

                std::vector<int> visibleIdx;
                for (int i = 0; i < 5; ++i)
                    if (counts[i] > 0) visibleIdx.push_back(i);

                int N = (int)visibleIdx.size();
                float startX = -((N - 1) * SPACING) / 2.0f;

                for (int i = 0; i < 5; ++i)
                {
                    if (!mp_PostGameBaseIcons[i]) continue;
                    mp_PostGameBaseIcons[i]->IsEnable = false;
                    if (baseTexts[i]) baseTexts[i]->IsEnable = false;
                }

                for (int j = 0; j < N; ++j)
                {
                    int i = visibleIdx[j];
                    float posX = startX + j * SPACING;

                    mp_PostGameBaseIcons[i]->IsEnable = true;
                    UiAnim::MoveTo(*this, mp_PostGameBaseIcons[i], { posX, BASE_Y }, 0.0f, Tweening::EasingType::Linear);

                    if (baseTexts[i])
                    {
                        baseTexts[i]->IsEnable = true;
                        baseTexts[i]->SetText(std::to_string(counts[i]));
                    }
                }
            }

            uint32_t crystalCounts[6] = {
                mp_Inventory->GetResourceCount<PurpleGemme>(),
                mp_Inventory->GetResourceCount<RedGemme>(),
                mp_Inventory->GetResourceCount<BlueGemme>(),
                mp_Inventory->GetResourceCount<GreenGemme>(),
                mp_Inventory->GetResourceCount<YellowGemme>(),
                mp_Inventory->GetResourceCount<OrangeGemme>(),
            };

            UiText* crystalTexts[6] = {
                mp_PostGameCrystalT1Text, mp_PostGameCrystalT2Text, mp_PostGameCrystalT3Text,
                mp_PostGameCrystalT4Text, mp_PostGameCrystalT5Text, mp_PostGameCrystalT6Text
            };

            //Idem pour les gemmes
            {
                constexpr float SPACING   = 110.0f;
                constexpr float CRYSTAL_Y = 330.0f;

                std::vector<int> visibleIdx;
                for (int i = 0; i < 6; ++i)
                    if (crystalCounts[i] > 0) visibleIdx.push_back(i);

                int N = (int)visibleIdx.size();
                float startX = -((N - 1) * SPACING) / 2.0f;

                for (int i = 0; i < 6; ++i)
                {
                    if (!mp_PostGameCrystalIcons[i]) continue;
                    mp_PostGameCrystalIcons[i]->IsEnable = false;
                    if (crystalTexts[i]) crystalTexts[i]->IsEnable = false;
                }

                for (int j = 0; j < N; ++j)
                {
                    int i = visibleIdx[j];
                    float posX = startX + j * SPACING;

                    mp_PostGameCrystalIcons[i]->IsEnable = true;
                    UiAnim::MoveTo(*this, mp_PostGameCrystalIcons[i], { posX, CRYSTAL_Y }, 0.0f, Tweening::EasingType::Linear);

                    if (crystalTexts[i])
                    {
                        crystalTexts[i]->IsEnable = true;
                        crystalTexts[i]->SetText(std::to_string(crystalCounts[i]));
                    }
                }
            }
        }

        if (mp_PostGameBtn1Sprite)
        {
            mp_PostGameBtn1Sprite->SetSprite("PostGame_Buttons", "ButtonUnSelected");
            mp_PostGameBtn1Sprite->SetScale({ 1.0f, 1.0f });
        }
        if (mp_PostGameBtn2Sprite)
        {
            mp_PostGameBtn2Sprite->SetSprite("PostGame_Buttons", "ButtonUnSelected");
            mp_PostGameBtn2Sprite->SetScale({ 1.0f, 1.0f });
        }

        if (mp_CharacterUi)      mp_CharacterUi->IsEnable      = false;
        if (mp_MaterialUi)       mp_MaterialUi->IsEnable       = false;
        if (mp_MaterialCrystalUi) mp_MaterialCrystalUi->IsEnable = false;
        if (mp_MiniMap)  mp_MiniMap->IsEnable  = false;
        if (mp_MiniMapBackground)  mp_MiniMapBackground->IsEnable = false;
        if (mp_PlayerMiniMapIndicator)  mp_PlayerMiniMapIndicator->IsEnable = false;
        if (mp_BackgroundAdminPanel)  mp_BackgroundAdminPanel->IsEnable = false;
        if (mp_BackgroundAdminPanel)  mp_BackgroundAdminPanel->IsEnable = false;
        if (mp_EnnemiGeneratorBehavior->GetBoss()->IsAlive) GetBehavior<BossBarHandler>(mp_EnnemiGeneratorBehavior->GetBoss()->EID)->DisableUI();

        if (mp_PlayerControllerBehavior) mp_PlayerControllerBehavior->SetEnabled(false);
        if (mp_PlayerControllerBehavior) mp_PlayerControllerBehavior->SetEnabled(false);
        if (mp_EnnemiGeneratorBehavior)  mp_EnnemiGeneratorBehavior->SetEnabled(false);
        if (mp_PlayerLivingBehavior)     mp_PlayerLivingBehavior->SetEnabled(false);
        if (mp_PlayerInteractionBehavior) mp_PlayerInteractionBehavior->SetEnabled(false);
        if (mp_IrisBehavior)              mp_IrisBehavior->SetEnabled(false);
        mp_PostGameUI->IsEnable = true;
        mp_ContinueButton->SetDisabled(!canContinue);
        mp_ContinueButtonText->IsEnable = canContinue;
        mp_TitleText->SetText(canContinue ? "ACCOMPLIE" : "VOUS AVEZ PERDU");

    }

    void SceneExpedition::UseFreeCamera() const {
        mp_MainCamera->Active = false;
        mp_FreeCameraCamera->Active = true;
        mp_MainCameraBehavior->SetEnabled(false);
        mp_FreeCameraBehavior->SetEnabled(true);
        mp_PlayerControllerBehavior->SetEnabled(false);
    }

    void SceneExpedition::UseFollowCamera() const {
        mp_MainCamera->Active = true;
        mp_FreeCameraCamera->Active = false;
        mp_MainCameraBehavior->SetEnabled(true);
        mp_FreeCameraBehavior->SetEnabled(false);
        mp_PlayerControllerBehavior->SetEnabled(true);
    }

    void SceneExpedition::SetupUI()
    {
        UiBuilder b(this);

        auto settingsEntity = CreateEntityAs2D();
        mp_SettingsBehaviour = AddBehavior<UISettings>(settingsEntity);

        mp_MainUI = b.At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
            .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
            .Tint(Colors::TRANSPARENT)
            .FitToScreen(true)
            .BuildCanvas();
        if (!mp_MainUI) return;
        const EntityID mainId = mp_MainUI->GetEntityID();

        /// CHARACTER UI
        {
            mp_CharacterUi = b.ChildOf(mainId)
                .At(Anchors::BOTTOM_LEFT, { 5.0f, -5.0f })
                .Size({ 600, 400 })
                .Tint(Colors::TRANSPARENT)
                .BuildCanvas();
            if (!mp_CharacterUi) return;
            const EntityID characterUiId = mp_CharacterUi->GetEntityID();

            // Cali
            UiSprite* caliHud = b.ChildOf(characterUiId)
                .At(Anchors::BOTTOM_LEFT, { 0.0f, 0.0f })
                .Scale({ kHudScale, kHudScale })
                .Sprite("626x213", "Cali_Hud")
                .BuildSprite();

            UiSprite* caliHp = b.ChildOf(caliHud->GetEntityID())
                .At(Anchors::BOTTOM_RIGHT, { 90.0f, 45.0f })
                .Scale({ kHudScale, kHudScale })
                .Sprite("626x213", "Cali_Hp")
                .BuildSprite();

            mp_PlayerHpText = b.ChildOf(caliHp->GetEntityID())
                .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::SNOW)
                .Text("100/100")
                .BuildText();
            if (mp_PlayerHpText) mp_PlayerHpText->BindReactive(m_PlayerHpReactive);

            // Iris
            UiSprite* irisHud = b.ChildOf(characterUiId)
                .At(Anchors::BOTTOM_LEFT, { 0.0f, -175.0f })
                .Scale({ kHudScale, kHudScale })
                .Sprite("300x139", "Iris_Hud")
                .BuildSprite();

            UiSprite* irisHp = b.ChildOf(irisHud->GetEntityID())
                .At(Anchors::BOTTOM_RIGHT, { 55.0f, 28.0f })
                .Scale({ kHudScale, kHudScale })
                .Sprite("300x139", "Iris_Hp")
                .BuildSprite();

            mp_IrisHpText = b.ChildOf(irisHp->GetEntityID())
                .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 3.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::SNOW)
                .Text("100/100")
                .BuildText();
            if (mp_IrisHpText) mp_IrisHpText->BindReactive(m_IrisHpReactive);
        }

        /// MATERIAL UI
        {
            mp_MaterialUi = b.ChildOf(mainId)
                .At(Anchors::BOTTOM_RIGHT, { -5.0f, -5.0f })
                .Layout(UiLayoutMode::Row)
                .Gap(10)
                .Padding(10)
                .Size({ 350, 50 })
                .Tint(Colors::TRANSPARENT)
                .BuildCanvas();
            if (!mp_MaterialUi) return;
            const EntityID materialUiId = mp_MaterialUi->GetEntityID();

            UiSprite* fiberUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Fiber")
                .FlexGrow(1)
                .BuildSprite();
            mp_FiberText = b.ChildOf(fiberUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("3")
                .BuildText();
            if (mp_FiberText) mp_FiberText->BindReactive(m_FiberReactive);

            UiSprite* woodUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Wood")
                .FlexGrow(1)
                .BuildSprite();
            mp_WoodText = b.ChildOf(woodUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("4")
                .BuildText();
            if (mp_WoodText) mp_WoodText->BindReactive(m_WoodReactive);

            UiSprite* foodUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Food")
                .FlexGrow(1)
                .BuildSprite();
            mp_FoodText = b.ChildOf(foodUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("5")
                .BuildText();
            if (mp_FoodText) mp_FoodText->BindReactive(m_FoodReactive);

            UiSprite* stoneUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Stone")
                .FlexGrow(1)
                .BuildSprite();
            mp_StoneText = b.ChildOf(stoneUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("6")
                .BuildText();
            if (mp_StoneText) mp_StoneText->BindReactive(m_StoneReactive);

            UiSprite* oreUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Ore")
                .FlexGrow(1)
                .BuildSprite();
            mp_OreText = b.ChildOf(oreUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("7")
                .BuildText();
            if (mp_OreText) mp_OreText->BindReactive(m_OreReactive);

            UiSprite* crystalUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Crystal")
                .FlexGrow(1)
                .BuildSprite();
            mp_CrystalText = b.ChildOf(crystalUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("8")
                .BuildText();
            if (mp_CrystalText) mp_CrystalText->BindReactive(m_CrystalReactive);
        }

        /// MATERIAL CRYSTAL UI
        {
            mp_MaterialCrystalUi = b.ChildOf(mainId)
                .At(Anchors::BOTTOM_RIGHT, { -20.0f, -55.0f })
                .Layout(UiLayoutMode::Column)
                .Gap(10)
                .Padding(10)
                .Size({ 50, 350 })
                .Tint(Colors::TRANSPARENT)
                .BuildCanvas();
            if (!mp_MaterialCrystalUi) return;
            mp_MaterialCrystalUi->IsEnable = false;
            const EntityID materialUiId = mp_MaterialCrystalUi->GetEntityID();

            UiSprite* fiberUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Crystal_T1")
                .FlexGrow(1)
                .BuildSprite();
            mp_CrystalT1Text = b.ChildOf(fiberUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("3")
                .BuildText();
            if (mp_CrystalT1Text) mp_CrystalT1Text->BindReactive(m_CrystalT1Reactive);

            UiSprite* woodUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Crystal_T2")
                .FlexGrow(1)
                .BuildSprite();
            mp_CrystalT2Text = b.ChildOf(woodUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("4")
                .BuildText();
            if (mp_CrystalT2Text) mp_CrystalT2Text->BindReactive(m_CrystalT2Reactive);

            UiSprite* foodUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Crystal_T3")
                .FlexGrow(1)
                .BuildSprite();
            mp_CrystalT3Text = b.ChildOf(foodUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("5")
                .BuildText();
            if (mp_CrystalT3Text) mp_CrystalT3Text->BindReactive(m_CrystalT3Reactive);

            UiSprite* stoneUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Crystal_T4")
                .FlexGrow(1)
                .BuildSprite();
            mp_CrystalT4Text = b.ChildOf(stoneUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("6")
                .BuildText();
            if (mp_CrystalT4Text) mp_CrystalT4Text->BindReactive(m_CrystalT4Reactive);

            UiSprite* oreUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Crystal_T5")
                .FlexGrow(1)
                .BuildSprite();
            mp_CrystalT5Text = b.ChildOf(oreUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("7")
                .BuildText();
            if (mp_CrystalT5Text) mp_CrystalT5Text->BindReactive(m_CrystalT5Reactive);

            UiSprite* crystalUi = b.ChildOf(materialUiId)
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Crystal_T6")
                .FlexGrow(1)
                .BuildSprite();
            mp_CrystalT6Text = b.ChildOf(crystalUi->GetEntityID())
                .At(Anchors::TOP_RIGHT, { 10.0f, 0.0f })
                .Font(UiTheme::DefaultFont, kHpTextFontSize)
                .Tint(Colors::LIGHT_GRAY)
                .Text("8")
                .BuildText();
            if (mp_CrystalT6Text) mp_CrystalT6Text->BindReactive(m_CrystalT6Reactive);
        }

        /// MATERIAL BTN
        {
            UiSprite* crystalBtn = b.ChildOf(mainId)
                .At(Anchors::BOTTOM_RIGHT, { -20.0f, -25.0f })
                .Scale({ kHudScale, kHudScale })
                .Sprite("49x47", "Hud_Crystal_Close")
                .BuildSprite();
            mp_CrystalButton = b.BuildButton(crystalBtn);
            if (mp_CrystalButton)
            {
                mp_CrystalButton->OnClick.ConnectLambda([this, crystalBtn]()
                {
                    if (mp_MaterialCrystalUi->IsEnable)
                    {
                        mp_MaterialCrystalUi->IsEnable = false;
                        crystalBtn->SetSprite("49x47", "Hud_Crystal_Close");
                    }
                    else
                    {
                        mp_MaterialCrystalUi->IsEnable = true;
                        crystalBtn->SetSprite("49x47", "Hud_Crystal_Open");
                    }
                });
            }
        }

        // SETTINGS
        {
            UiSprite* toggle = b.ChildOf(mainId)
                                .At(Anchors::TOP_RIGHT, { 0.0f, 0.0f })
                                .Sprite("218x169", "Button_Settings")
                                .BuildSprite();

            UiButton* button = b.BuildButton(toggle);
            if (!button) return;
            button->OnClick.ConnectLambda([this]()
            {
                settingsIsVisible = !settingsIsVisible;
                if (mp_SettingsBehaviour) mp_SettingsBehaviour->Toggle(settingsIsVisible);
            });

            if (mp_SettingsBehaviour)
            {
                mp_SettingsBehaviour->OnSettingsChanged.ConnectLambda([this]() {
                    SettingsGame::Get().Save();

                    const float musicVol = SettingsGame::Get().GetVolume(AudioType::Music);
                    const float sfxVol   = SettingsGame::Get().GetVolume(AudioType::SFX);

                    mp_ExploreMusic->Volume = musicVol;
                    mp_FightMusic->Volume   = musicVol;
                    mp_UIpopSfx->Volume     = sfxVol;

                    for (auto* src : m_FootstepPlayer.Sources)    if (src) src->Volume = sfxVol;
                    for (auto* src : m_SlidePlayer.Sources)       if (src) src->Volume = sfxVol;
                    for (auto* src : m_MiningStonePlayer.Sources) if (src) src->Volume = sfxVol;
                    for (auto* src : m_MiningWoodPlayer.Sources)  if (src) src->Volume = sfxVol;
                    for (auto* src : m_AttackPlayer.Sources)      if (src) src->Volume = sfxVol;
                    for (auto* src : m_DamageEnemie.Sources)      if (src) src->Volume = sfxVol;
                    for (auto* src :m_WalkEnemie.Sources)         if (src) src->Volume = sfxVol;
                });

                mp_SettingsBehaviour->OnCloseRequested.ConnectLambda([this]()
                {
                    settingsIsVisible = !settingsIsVisible;
                    // TODO : mp_UIpopClose->Play();
                });
            }
        }

        SetupPostGameUI();

        SetUpAdminPannel();
    }

    void SceneExpedition::SetupAudio()
    {
        mp_ExploreMusic = AddComponent<AudioSource>(CreateEntityAs3D());
        mp_ExploreMusic->Path = "Resources/Audio/Music/Ruins_of_hestia_explore.wav";
        mp_ExploreMusic->Resource = GPC_AUDIO("ROH_explore");
        mp_ExploreMusic->TypeVoice = AudioType::Music;
        mp_ExploreMusic->Volume = SettingsGame::Get().GetVolume(AudioType::Music);
        mp_ExploreMusic->Looping = true;
        mp_ExploreMusic->Play();

        mp_FightMusic = AddComponent<AudioSource>(CreateEntityAs3D());
        mp_FightMusic->Path = "Resources/Audio/Music/Ruins_of_hestia_fight.wav";
        mp_FightMusic->Resource = GPC_AUDIO("ROH_fight");
        mp_FightMusic->TypeVoice = AudioType::Music;
        mp_FightMusic->Volume = SettingsGame::Get().GetVolume(AudioType::Music);
        mp_FightMusic->Looping = true;

        mp_UIpopSfx = AddComponent<AudioSource>(CreateEntityAs3D());
        mp_UIpopSfx->Path = "Resources/Audio/SFX/sfx_ui4.wav";
        mp_UIpopSfx->Resource = GPC_AUDIO("UI4");
        mp_UIpopSfx->TypeVoice = AudioType::SFX;
        mp_UIpopSfx->Volume = SettingsGame::Get().GetVolume(AudioType::SFX);
        mp_UIpopSfx->Looping = false;

        mp_WinSFX = AddComponent<AudioSource>(CreateEntityAs3D());
        mp_WinSFX->Path = "Resources/Audio/SFX/sfx_victory.wav";
        mp_WinSFX->Resource = GPC_AUDIO("Victory");
        mp_WinSFX->TypeVoice = AudioType::SFX;
        mp_WinSFX->Volume = SettingsGame::Get().GetVolume(AudioType::SFX);
        mp_WinSFX->Looping = false;

        auto makeSource = [&](const char* resource, const char* path) -> AudioSource*
        {
            auto* src = AddComponent<AudioSource>(CreateEntityAs3D());
            src->Path      = path;
            src->Resource  = GPC_AUDIO(resource);
            src->TypeVoice = AudioType::SFX;
            src->Volume    = SettingsGame::Get().GetVolume(AudioType::SFX);
            src->Looping   = false;
            return src;
        };

        m_FootstepPlayer.Sources = {
            makeSource("Chemin1", "Resources/Audio/SFX/bruits de pas/chemin1.wav"),
            makeSource("Chemin2", "Resources/Audio/SFX/bruits de pas/chemin2.wav"),
            makeSource("Chemin3", "Resources/Audio/SFX/bruits de pas/chemin3.wav"),
            makeSource("Chemin4", "Resources/Audio/SFX/bruits de pas/chemin4.wav"),
            makeSource("Chemin5", "Resources/Audio/SFX/bruits de pas/chemin5.wav"),
            makeSource("Chemin6", "Resources/Audio/SFX/bruits de pas/chemin6.wav"),
        };

        m_SlidePlayer.Sources = {
            makeSource("Slide1", "Resources/Audio/SFX/EXPEDITION/dash_1.wav"),
            makeSource("Slide2", "Resources/Audio/SFX/EXPEDITION/dash_2en1.wav"),
            makeSource("Slide3", "Resources/Audio/SFX/EXPEDITION/dash_3.wav"),
        };

        m_MiningStonePlayer.Sources = {
            makeSource("MiningStone1", "Resources/Audio/SFX/EXPEDITION/mining_stone_1.wav"),
            makeSource("MiningStone2", "Resources/Audio/SFX/EXPEDITION/mining_stone_2.wav"),
            makeSource("MiningStone3", "Resources/Audio/SFX/EXPEDITION/mining_stone_3.wav"),
        };

        m_MiningWoodPlayer.Sources = {
            makeSource("MiningWood1", "Resources/Audio/SFX/EXPEDITION/mining_wood_1.wav"),
            makeSource("MiningWood2", "Resources/Audio/SFX/EXPEDITION/mining_wood_2.wav"),
            makeSource("MiningWood3", "Resources/Audio/SFX/EXPEDITION/mining_wood_3.wav"),
        };

        m_AttackPlayer.Sources = {
            makeSource("Attack1", "Resources/Audio/SFX/EXPEDITION/metal_glove_hit_1.wav"),
            makeSource("Attack2", "Resources/Audio/SFX/EXPEDITION/metal_glove_hit_2.wav"),
            makeSource("Attack3", "Resources/Audio/SFX/EXPEDITION/metal_glove_hit_3.wav"),
        };

        m_DamageEnemie.Sources = {
            makeSource("EnemiCry1","Resources/Audio/SFX/ENNEMIE/monster_cry_1.wav"),
            makeSource("EnemiCry2","Resources/Audio/SFX/ENNEMIE/monster_cry_2.wav"),
            makeSource("EnemiCry3","Resources/Audio/SFX/ENNEMIE/monster_cry_3.wav"),
            makeSource("EnemiCry4", "Resources/Audio/SFX/ENNEMIE/monster_cry_4.wav")
        };

        m_WalkEnemie.Sources = {
            makeSource("EnemiFoot1","Resources/Audio/SFX/ENNEMIE/monster_footstep_1.wav"),
            makeSource("EnemiFoot2","Resources/Audio/SFX/ENNEMIE/monster_footstep_2.wav"),
            makeSource("EnemiFoot3","Resources/Audio/SFX/ENNEMIE/monster_footstep_3.wav"),
            makeSource("EnemiFoot4", "Resources/Audio/SFX/ENNEMIE/monster_footstep_4.wav")
        };
    }

    void SceneExpedition::SetUpAdminPannel() {

        UiBuilder builder{this};

        mp_BackgroundAdminPanel = builder
            .ChildOf(mp_MainUI->GetEntityID())
            .At(Anchors::TOP_LEFT, { 500.0f, -150.0f })
            .Scale(0.2f)
            .Sprite("1443x998", "Settings_Background")
            .BuildSprite();

        // Open / Close
        {
            UiText* open_close = builder.ChildOf(mp_BackgroundAdminPanel->GetEntityID())
                         .At(Anchors::BOTTOM_MIDDLE, { 0.0f, -10.0f })
                         .FontSize(25.0f)
                         .Tint(Colors::PERFECT_BROWN)
                         .Text("Toggle Admin Panel")
                         .BuildText();

            UiButton* open_close_btn = builder.BuildButton(open_close);
            if (open_close_btn)
            {
                open_close_btn->OnClick.ConnectLambda([this]()
                {

                    if (m_AdminPannelIsOpen) {
                        UiAnim::MoveTo(*this, mp_BackgroundAdminPanel, { 500.0f, -150.0f }, 0.2f, Tweening::EasingType::EaseInBounce);
                        m_AdminPannelIsOpen = false;
                    }
                    else {
                        UiAnim::MoveTo(*this, mp_BackgroundAdminPanel, { 500.0f, -15.0f }, 0.2f, Tweening::EasingType::EaseInBounce);
                        m_AdminPannelIsOpen = true;
                    }

                });
            }
        }

        // Unstuck
        {
            UiText* unstuck = builder.ChildOf(mp_BackgroundAdminPanel->GetEntityID())
                     .At(Anchors::TOP_LEFT, { 18.0f, 35.0f })
                     .FontSize(25.0f)
                     .Tint(Colors::PERFECT_BROWN)
                     .Text("Unstuck")
                     .BuildText();

            UiButton* unstuck_btn = builder.BuildButton(unstuck);
            if (unstuck_btn)
            {
                unstuck_btn->OnClick.ConnectLambda([this]()
                {
                    auto pGeneration = GPC::MapGeneratorFactory::GetGeneration(m_GenerationID);

                    glm::ivec2 room = WorldPositionToRoomPosition(mp_PlayerTransform->GetWorldPosition());

                    for (auto data : pGeneration->GeneratedRoomDatas) {
                        if (data.MapPosition.x == room.x && data.MapPosition.y == room.y) {

                            glm::vec2 input = { ROOM_SIZE - data.pMeta->GetPlayer().position.x - 0.5f, data.pMeta->GetPlayer().position.y + 0.5f };
                            glm::vec2 centered_input = input - glm::vec2{ROOM_SIZE, ROOM_SIZE} * 0.5f;
                            glm::vec2 rotated_input = pGeneration->GeneratedMissionData.GetRotation(data.MapPosition.x, data.MapPosition.y) * centered_input;
                            glm::vec2 normalized_input = 2.0f * rotated_input / glm::vec2(ROOM_SIZE, ROOM_SIZE);

                            glm::vec2 position = {
                                MapGeneratorData::ROOM_SCALE * (static_cast<float>(data.MapPosition.x) + 0.5f) + ROOM_SIZE * normalized_input.x * MapGeneratorData::TILE_GROUP_SCALE * 0.5f,
                                MapGeneratorData::ROOM_SCALE * (static_cast<float>(data.MapPosition.y) + 0.5f) + ROOM_SIZE * normalized_input.y * MapGeneratorData::TILE_GROUP_SCALE * 0.5f
                            };

                            mp_PlayerTransform->LocalTransform.SetPosition(position.x, mp_PlayerTransform->LocalTransform.GetScale().y * 0.5f, position.y);

                            break;
                        }
                    }
                });
            }
        }

        // Skip Expedition
        {

            UiText* skip_expedition = builder.ChildOf(mp_BackgroundAdminPanel->GetEntityID())
                     .At(Anchors::TOP_LEFT, { 18.0f, 65.0f })
                     .FontSize(25.0f)
                     .Tint(Colors::PERFECT_BROWN)
                     .Text("Go To End Of Expedition")
                     .BuildText();

            UiButton* skip_expedition_btn = builder.BuildButton(skip_expedition);
            if (skip_expedition_btn)
            {
                skip_expedition_btn->OnClick.ConnectLambda([this]()
                {
                    SetPlayerAtEnd();
                });
            }
        }

        // Heal
        {

            UiText* heal = builder.ChildOf(mp_BackgroundAdminPanel->GetEntityID())
                     .At(Anchors::TOP_LEFT, { 18.0f, 95.0f })
                     .FontSize(25.0f)
                     .Tint(Colors::PERFECT_BROWN)
                     .Text("Heal Cali")
                     .BuildText();

            UiButton* heal_btn = builder.BuildButton(heal);
            if (heal_btn)
            {
                heal_btn->OnClick.ConnectLambda([this]()
                {
                    mp_PlayerLivingBehavior->CurrentHP = mp_PlayerLivingBehavior->pMaxHealth->GetFinalValue();
                });
            }
        }

        // Teleport to Boss
        {

            UiText* tp_to_boss = builder.ChildOf(mp_BackgroundAdminPanel->GetEntityID())
                     .At(Anchors::TOP_LEFT, { 18.0f, 125.0f })
                     .FontSize(25.0f)
                     .Tint(Colors::PERFECT_BROWN)
                     .Text("Teleport to boss")
                     .BuildText();

            UiButton* tp_to_boss_btn = builder.BuildButton(tp_to_boss);
            if (tp_to_boss_btn)
            {
                tp_to_boss_btn->OnClick.ConnectLambda([this]()
                {
                    mp_PlayerTransform->LocalTransform.SetPosition(mp_EnnemiGeneratorBehavior->GetBoss()->pTransform->LocalTransform.GetPosition());
                });
            }
        }

    }

    void SceneExpedition::SetupBossBar() {

    }

    void SceneExpedition::StartMission() {
        GenerateMap();
        GenerateNoise();

        CreateTerrain();
        CreateEnnemies();
        CreateResources();
        CreateEndOfMission();

        SetPlayerAtSpawnPoint();
        // mp_IrisTransform->LocalTransform.SetPosition(mp_PlayerTransform->LocalTransform.GetPosition());

        if (mp_Inventory != nullptr)
        {
            mp_Inventory->Clear();
        }
        
        if (mp_MiniMap != nullptr) {
            auto pGeneration = GPC::MapGeneratorFactory::GetGeneration(m_GenerationID);
            mp_MiniMap->SetSprite(pGeneration->GetArrayName(), pGeneration->GetTextureName());
        }
    }

    void SceneExpedition::DestroyMission() {

        if (mp_TerrainGeneratorBehavior != nullptr) {
            mp_TerrainGeneratorBehavior->DestroyTerrain(this);
            DestroyEntity(mp_TerrainGeneratorBehavior->GetEntityID());
            mp_TerrainGeneratorBehavior = nullptr;
        }

        if (mp_EnnemiGeneratorBehavior != nullptr) {
            mp_EnnemiGeneratorBehavior->DestroyAllEnnemies(this);
            mp_EnnemiGeneratorBehavior->pEnemiWalk = &m_WalkEnemie;
            DestroyEntity(mp_EnnemiGeneratorBehavior->GetEntityID());
            mp_EnnemiGeneratorBehavior = nullptr;
        }

        if (mp_ResourceGeneratorBehavior != nullptr) {
            mp_ResourceGeneratorBehavior->DestroyAllResources(this);
            DestroyEntity(mp_ResourceGeneratorBehavior->GetEntityID());
            mp_ResourceGeneratorBehavior = nullptr;
        }

        DestroyEntity(mp_EndOfMissionTransform->GetEntityID());

    }

    void SceneExpedition::DestroyAllWithAsteroid() {

        auto pauvre_entities = m_EntityManager.GetLivingEntities();
        auto qte_pauvre_entities = m_EntityManager.GetLivingEntitiesCount();
        for (uint32_t i = 0; i < qte_pauvre_entities; ++i) {
            DestroyEntity(pauvre_entities[i]);
        }

    }

    void SceneExpedition::LoadAllRooms() {
        constexpr uint16_t MAX_ROOM_COUNT = 80;
        m_RoomMetas.resize(MAX_ROOM_COUNT);
        uint32_t count = 0;
        for (uint32_t i = 0; i < MAX_ROOM_COUNT; ++i) {
            std::string path = "Resources/RoomData/Room" + std::to_string(i) + ".room";
            if (std::filesystem::exists(path) == false) break;
            m_RoomMetas[i].LoadFromFile(path);
            m_RoomMetas[i].FILE = path;
            count++;
            GPC_INFO << "Room Loaded : " << path << ENDL;
        }
        m_RoomMetas.resize(count);
    }

    void SceneExpedition::GenerateMap() {
        m_GenerationID = GPC::MapGeneratorFactory::CreateNew();
        auto pGeneration = GPC::MapGeneratorFactory::GetGeneration(m_GenerationID);
        pGeneration->GenerateWithRoom(GPC::MapMask::CreateAsCircle(1.1f), m_RoomMetas);
    }

    void SceneExpedition::GenerateNoise() {
        m_UniformTextureIDs.TextureCount = { 8, 8, 8, 8 };
        for (auto & texID : m_UniformTextureIDs.TextureIDs) {
            texID.x = rand() % 8;
            texID.y = rand() % 8;
            texID.z = rand() % 8;
            texID.w = rand() % 8;
        }

        constexpr uint16_t ColorCount = 4;
        glm::vec4 GroundColor[ColorCount] {
            pow(static_cast<glm::vec4>(GPC::Color(0xD98D37FF)), glm::vec4(3.0f)),
            pow(static_cast<glm::vec4>(GPC::Color(0xE57041FF)), glm::vec4(3.0f)),
            pow(static_cast<glm::vec4>(GPC::Color(0xDE6D60FF)), glm::vec4(3.0f)),
            pow(static_cast<glm::vec4>(GPC::Color(0xB3608FFF)), glm::vec4(3.0f)),
        };

        for (auto & color : m_UniformTextureIDs.RoomColors) {
            color = GroundColor[rand() % ColorCount];
        }
    }

    void SceneExpedition::CreatePlayer() {
        auto player = CreateEntityAs3D();

        mp_PlayerTransform     = GetComponent<Transform3D>(player);
        mp_PlayerTransform->LocalTransform.SetScale(0.0015 * GENERAL_SCALE_EXPEDITION, 0.0015 * GENERAL_SCALE_EXPEDITION, 0.0015 * GENERAL_SCALE_EXPEDITION);
        mp_PlayerTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI, 0, 0);

        auto pCollider      = AddComponent<Collision3DComponent>(player);
        pCollider->IsSphere = false;
        pCollider->IsTrigger = false;
        pCollider->CollType = ColliderType::Player;

        auto pRenderer      = AddComponent<Render3DComponent>(player);
        pRenderer->pGeometry = GPC_MESH("Cali_Running");
        pRenderer->SetTexture("2048x2048", "CaliTexture");
        pRenderer->UseShadow = true;

        auto pAnimation  = AddComponent<AnimationComponent>(player);
        pAnimation->SetFbxData(mp_CaliFbxData);
        pAnimation->StackIndex = 1;
        pAnimation->Playing = true;
        pAnimation->Looping = true;
        pAnimation->Speed = 1.83f;

        auto pTween = AddComponent<TweenComponent>(player);

        mp_PlayerControllerBehavior     = AddBehavior<PlayerController>(player);
        mp_PlayerInteractionBehavior    = AddBehavior<PlayerInteraction>(player);
        mp_PlayerLivingBehavior         = AddBehavior<LivingEntityBehavior>(player);
        mp_PlayerLivingBehavior->InitAsCali();

        mp_PlayerControllerBehavior->pCaliFbxData        = mp_CaliFbxData;
        mp_PlayerControllerBehavior->pSlidePlayer        = &m_SlidePlayer;
        mp_PlayerControllerBehavior->pFootstepPlayer     = &m_FootstepPlayer;
        mp_PlayerInteractionBehavior->pMiningStonePlayer = &m_MiningStonePlayer;
        mp_PlayerInteractionBehavior->pMiningWoodPlayer  = &m_MiningWoodPlayer;
        mp_PlayerInteractionBehavior->pAttackPlayer      = &m_AttackPlayer;
        mp_PlayerInteractionBehavior->pDamageEnemie      = &m_DamageEnemie;
        mp_PlayerInteractionBehavior->pLiving            = mp_PlayerLivingBehavior;
        mp_PlayerInteractionBehavior->m_UseTuto          = m_UseTuto;

        mp_Helmet = new Helmet(1);
        mp_Helmet->OnBaseStatistics();
        mp_Helmet->ReRollTraits();
        mp_PlayerLivingBehavior->EquipHelmet(mp_Helmet);

        mp_Melee = new ShortSword(1);
        mp_Melee->OnBaseStatistics();
        mp_Melee->ReRollTraits();
        mp_PlayerLivingBehavior->EquipMelee(mp_Melee);

        // auto iris = CreateEntityAs3D();
        // mp_IrisTransform = GetComponent<Transform3D>(iris);
        // mp_IrisBehavior = AddBehavior<IrisFollowing>(iris);
        // mp_IrisBehavior->pPlayerTransform = mp_PlayerTransform;
        // mp_IrisBehavior->pIrisMesh = mp_IrisFbxData;
    }

    void SceneExpedition::CreateTerrain() {
        auto map    = CreateEntityAs3D();
        mp_TerrainGeneratorBehavior = AddBehavior<TerrainGeneratorBehavior>(map);
        mp_TerrainGeneratorBehavior->GenerationID = m_GenerationID;
        mp_TerrainGeneratorBehavior->pPlayerTransform = mp_PlayerTransform;
        mp_TerrainGeneratorBehavior->pProps = &mp_Props;
    }

    void SceneExpedition::CreateEnnemies() {
        auto ennemi_spawners = CreateEntityAs3D();
        mp_EnnemiGeneratorBehavior = AddBehavior<EnnemiGeneratorBehavior>(ennemi_spawners);
        mp_EnnemiGeneratorBehavior->GenerationID = m_GenerationID;
        mp_EnnemiGeneratorBehavior->pPlayerTransform = mp_PlayerTransform;
        mp_EnnemiGeneratorBehavior->Difficulty = m_Difficulty;
        mp_EnnemiGeneratorBehavior->pMeshData = mp_EnnemiFbxData;
        mp_EnnemiGeneratorBehavior->pMainCanvas = mp_MainUI;
        mp_PlayerInteractionBehavior->pEnnemiManager = mp_EnnemiGeneratorBehavior;
        mp_PlayerLivingBehavior->pEnnemiManager = mp_EnnemiGeneratorBehavior;
    }

    void SceneExpedition::CreateResources() {
        auto resource_spawners = CreateEntityAs3D();
        mp_ResourceGeneratorBehavior = AddBehavior<ResourceGeneratorBehavior>(resource_spawners);
        mp_ResourceGeneratorBehavior->GenerationID = m_GenerationID;
        mp_ResourceGeneratorBehavior->pPlayerTransform = mp_PlayerTransform;
        mp_ResourceGeneratorBehavior->pInventory = mp_Inventory;
        mp_ResourceGeneratorBehavior->pCanvas = mp_MainUI;
        mp_PlayerInteractionBehavior->pResourceManager = mp_ResourceGeneratorBehavior;
    }

    void SceneExpedition::CreateEndOfMission() {
        auto end_of_mission = CreateEntityAs3D();

        constexpr float END_OF_MISSION = 2.0f;
        mp_EndOfMissionTransform = GetComponent<Transform3D>(end_of_mission);
        mp_EndOfMissionTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 1.25f, 0, 0);
        mp_EndOfMissionTransform->LocalTransform.SetScale(
            END_OF_MISSION * GENERAL_SCALE_EXPEDITION,
            END_OF_MISSION * GENERAL_SCALE_EXPEDITION,
            END_OF_MISSION * GENERAL_SCALE_EXPEDITION);
        SetEndOfMissionPoint();

        auto eomRenderer = AddComponent<Render3DComponent>(end_of_mission);
        eomRenderer->pGeometry = GPC_MESH("Plane");
        eomRenderer->SetTexture("1172x745", "CampFire");

        auto eomCollider = AddComponent<Collision3DComponent>(end_of_mission);
        eomCollider->IsSphere = false;
        eomCollider->IsTrigger = true;

        auto eomBehavior = AddBehavior<BackToVillageBehavior>(end_of_mission);
        eomBehavior->pPlayerTransform = mp_PlayerTransform;
        eomBehavior->pInventory = mp_Inventory;
        eomBehavior->UseTuto = m_UseTuto;
        eomBehavior->Start_Time = m_StartTime;

        eomBehavior->OnInteract = [this]()
        {
           ShowPostGameUI(m_Difficulty);
        };
    }

    void SceneExpedition::CreateFreeCamera() {
        auto camera_libre = CreateEntityAs3D();

        auto clTransform    = GetComponent<Transform3D>(camera_libre);
        clTransform->LocalTransform.SetPosition(0.0f, 50.0f, 0.0f);
        clTransform->LocalTransform.SetRotationYawPitchRoll(0, glm::radians(-45.0f), 0);

        mp_FreeCameraCamera = AddComponent<CameraComponent>(camera_libre);

        mp_FreeCameraBehavior = AddBehavior<FreeCamera>(camera_libre);
        mp_FreeCameraBehavior->MovementSpeed = 30.0f * GENERAL_SCALE_EXPEDITION;
    }

    void SceneExpedition::CreatePlayerCamera() {
        constexpr float CAMERA_DISTANCE = 100.0f * GENERAL_SCALE_EXPEDITION;
        mp_MainCamera->UseOrthographic(1.5f * GENERAL_SCALE_EXPEDITION);

        mp_MainCameraTransform->LocalTransform.SetRotationYawPitchRoll({glm::radians(225.0f), glm::radians(-45.0f), 0});
        mp_MainCameraTransform->LocalTransform.SetPosition(mp_PlayerTransform->GetWorldPosition() + mp_MainCameraTransform->LocalTransform.GetForward() * CAMERA_DISTANCE);

        mp_MainCameraBehavior = AddBehavior<FollowTarget>(mp_MainCamera->GetEntityID());
        mp_MainCameraBehavior->LerpSpeed = 8.0f;
        mp_MainCameraBehavior->pTarget = GetComponent<Transform3D>(mp_PlayerControllerBehavior->GetEntityID());
        mp_MainCameraBehavior->Offset = mp_MainCameraTransform->LocalTransform.GetForward() * CAMERA_DISTANCE + glm::vec3( 3.0f, 0.0f, 3.0f );
    }

    void SceneExpedition::CreateSunLighting() {
        auto lTransform = GetComponent<Transform3D>(mp_DefaultLight->GetEntityID());
        lTransform->LocalTransform.SetRotationYawPitchRoll( glm::radians(10.0f), glm::radians(60.0f), 0 );
    }

    void SceneExpedition::CreateWater() {
        constexpr uint16_t WATER_TILING = 14;
        constexpr float WATER_TILE_SIZE = MAX_MAP_SIZE * GPC::MapGeneratorData::ROOM_SCALE * 1.2f / WATER_TILING;

        for (uint32_t i = 0; i < WATER_TILING; ++i) {
            for (uint32_t j = 0; j < WATER_TILING; ++j) {
                auto water = CreateEntityAs3D();

                auto wTransform = GetComponent<Transform3D>(water);
                wTransform->LocalTransform.SetScale(WATER_TILE_SIZE, 1, WATER_TILE_SIZE);
                wTransform->LocalTransform.SetPosition(
                    i * WATER_TILE_SIZE,
                    -1.5f * GENERAL_SCALE_EXPEDITION,
                    j * WATER_TILE_SIZE
                );

                auto wRenderer = AddComponent<Render3DComponent>(water);
                wRenderer->SetTexture("2048x2048", "Water");
                wRenderer->pGeometry = GPC_MESH("Plane");
            }
        }
    }

    void SceneExpedition::CreateMiniMap() {
        UiBuilder builder{this};

        m_MiniMapPosition = { 10.0f, 10.0f };
        m_MiniMapScale    = 0.75f;
        m_MiniMapRotation = GPC_PI * 0.25f;

        mp_MiniMapBackground = builder
            .ChildOf(mp_MainUI->GetEntityID())
            .At(Anchors::TOP_LEFT, m_MiniMapPosition)
            .Scale(0.375f)
            .Sprite("512x512", "BG_MAP")
            .BuildSprite();

        mp_MiniMap = builder
            .ChildOf(mp_MiniMapBackground->GetEntityID())
            .At(Anchors::MIDDLE_MIDDLE, {0.0f, 0.0f})
            .Scale(m_MiniMapScale)
            .Sprite("512x512", "BG_MAP")
            .BuildSprite();
        mp_MiniMap->Rotation = m_MiniMapRotation;

        mp_PlayerMiniMapIndicator = builder
            .ChildOf(mp_MiniMapBackground->GetEntityID())
            .At(Anchors::TOP_LEFT, {0.0f, 0.0f})
            .Scale(m_MiniMapScale * 0.02f)
            .Sprite("512x512", "PLAYER_MAP")
            .BuildSprite();

    }

    void SceneExpedition::SetPlayerAtSpawnPoint() {
        auto pGeneration = GPC::MapGeneratorFactory::GetGeneration(m_GenerationID);
        for (auto data : pGeneration->GeneratedRoomDatas) {
            if (data.MapPosition.x == pGeneration->GeneratedLayout.GetStart().x && data.MapPosition.y == pGeneration->GeneratedLayout.GetStart().y) {

                glm::vec2 input = { ROOM_SIZE - data.pMeta->GetPlayer().position.x - 0.5f, data.pMeta->GetPlayer().position.y + 0.5f };
                glm::vec2 centered_input = input - glm::vec2{ROOM_SIZE, ROOM_SIZE} * 0.5f;
                glm::vec2 rotated_input = pGeneration->GeneratedMissionData.GetRotation(data.MapPosition.x, data.MapPosition.y) * centered_input;
                glm::vec2 normalized_input = 2.0f * rotated_input / glm::vec2(ROOM_SIZE, ROOM_SIZE);

                glm::vec2 position = {
                    MapGeneratorData::ROOM_SCALE * (static_cast<float>(data.MapPosition.x) + 0.5f) + ROOM_SIZE * normalized_input.x * MapGeneratorData::TILE_GROUP_SCALE * 0.5f,
                    MapGeneratorData::ROOM_SCALE * (static_cast<float>(data.MapPosition.y) + 0.5f) + ROOM_SIZE * normalized_input.y * MapGeneratorData::TILE_GROUP_SCALE * 0.5f
                };

                mp_PlayerTransform->LocalTransform.SetPosition(position.x, mp_PlayerTransform->LocalTransform.GetScale().y * 0.5f, position.y);

                break;
            }
        }
    }

    void SceneExpedition::SetEndOfMissionPoint() {
        auto pGeneration = GPC::MapGeneratorFactory::GetGeneration(m_GenerationID);
        for (auto data : pGeneration->GeneratedRoomDatas) {
            if (data.MapPosition.x == pGeneration->GeneratedLayout.GetEnd().x && data.MapPosition.y == pGeneration->GeneratedLayout.GetEnd().y) {

                glm::vec2 input = { ROOM_SIZE - data.pMeta->GetPlayer().position.x - 0.5f, data.pMeta->GetPlayer().position.y + 0.5f };
                glm::vec2 centered_input = input - glm::vec2{ROOM_SIZE, ROOM_SIZE} * 0.5f;
                glm::vec2 rotated_input = pGeneration->GeneratedMissionData.GetRotation(data.MapPosition.x, data.MapPosition.y) * centered_input;
                glm::vec2 normalized_input = 2.0f * rotated_input / glm::vec2(ROOM_SIZE, ROOM_SIZE);

                glm::vec2 position = {
                    MapGeneratorData::ROOM_SCALE * (static_cast<float>(data.MapPosition.x) + 0.5f) + ROOM_SIZE * normalized_input.x * MapGeneratorData::TILE_GROUP_SCALE * 0.5f,
                    MapGeneratorData::ROOM_SCALE * (static_cast<float>(data.MapPosition.y) + 0.5f) + ROOM_SIZE * normalized_input.y * MapGeneratorData::TILE_GROUP_SCALE * 0.5f
                };

                mp_EndOfMissionTransform->LocalTransform.SetPosition(
                    position.x,
                    0.01f,
                    position.y
                );

                break;
            }
        }
    }

    void SceneExpedition::SetPlayerAtEnd() {
        glm::vec3 end_pos = mp_EndOfMissionTransform->GetWorldPosition();
        mp_PlayerTransform->LocalTransform.SetPosition(
            end_pos.x,
            mp_PlayerTransform->LocalTransform.GetScale().y * 0.5f,
            end_pos.z
        );

        if (mp_MainCameraBehavior && mp_MainCameraBehavior->pTarget && mp_MainCameraTransform)
        {
            const glm::vec3 target = mp_MainCameraBehavior->pTarget->LocalTransform.GetPosition()
                                   + mp_MainCameraBehavior->Offset;
            mp_MainCameraTransform->LocalTransform.SetPosition(target);
        }
    }

    ErrorType SceneExpedition::OnAssetsLoad() {
        GPC_SUCCESS(SceneDefault::OnAssetsLoad());

         // Program Loading
        {
            GPC_ASSETS->AddShader("Map Vertex", "map.vert", Shader::VERTEX);
            GPC_ASSETS->AddShader("Map Fragment", "map.frag", Shader::FRAGMENT);

            GPC_ASSETS->AddGraphicProgram(
                "Map Program",
                GPC_SHADER("Map Vertex"),
                GPC_SHADER("Map Fragment"),
                CullMode::FRONT,
                PolygonMode::MODE_FILL,
                {
                    {
                        .BindingDescription = GPC::Vertex::GetBindingDescription(),
                        .AttributeDescriptions = GPC::Vertex::GetAttributeDescriptions()
                    }
                },
                {
                    GPC::DescriptorSetInformation::ForCamera(0),
                    GPC::DescriptorSetInformation::ForTransform(1),
                    GPC::DescriptorSetInformation::ForMaterials(2),
                    GPC::DescriptorSetInformation::ForLights(3),
                    GPC::DescriptorSetInformation {
                         .Name = "images",
                         .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                         .BindingSlot = 4,
                         .DescriptorCount = 50,
                         .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                         .Sampler_ = nullptr
                    },
                    GPC::DescriptorSetInformation {
                        .Name = "LightTransform",
                        .Type = DescriptorType::UNIFORM_BUFFER,
                        .Usages = UNIFORM_BUFFER_BIT,
                        .BindingSlot = 5,
                        .DescriptorCount = 1,
                        .ElementCount = 1,
                        .ElementStride = sizeof(TransformLight),
                        .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .Sampler_ = nullptr
                    },
                    GPC::DescriptorSetInformation {
                         .Name = "shadowSpotLightMapDepth",
                         .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                         .BindingSlot = 6,
                         .DescriptorCount = 1,
                         .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                         .Sampler_ = nullptr
                    },
                    GPC::DescriptorSetInformation {
                         .Name = "shadowPointLightMapDepth",
                         .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                         .BindingSlot = 7,
                         .DescriptorCount = 1,
                         .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                         .Sampler_ = nullptr,
                    },
                    GPC::DescriptorSetInformation {
                         .Name = "shadowDirLightMapDepth",
                         .Type = GPC::DescriptorType::COMBINED_IMAGE_SAMPLER,
                         .BindingSlot = 8,
                         .DescriptorCount = 1,
                         .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                         .Sampler_ = nullptr,
                    },
                    GPC::DescriptorSetInformation {
                        .Name = "UniformRoomTileTextureID",
                        .Type = DescriptorType::UNIFORM_BUFFER,
                        .Usages = UNIFORM_BUFFER_BIT,
                        .BindingSlot = 9,
                        .DescriptorCount = 1,
                        .ElementCount = 1,
                        .ElementStride = sizeof(UniformTextureID),
                        .Flags = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .Sampler_ = nullptr
                    },
                },
                GRAPHICS_3D,
                mp_Window
            );

            auto& program = GPC_GRAPHIC_PROGRAM("Map Program")->Program;
            program.InitializeWith({&GPC_TEXTURE_ARRAY("1024x1024")->Array}, GPC_TEXTURE_ARRAY("1024x1024")->TextureSampler);
            // program.InitializeWith({&GPC_TEXTURE_ARRAY("MapTexture")->Array}, GPC_TEXTURE_ARRAY("MapTexture")->TextureSampler);
            program.UseShadows(mp_Window);

        }

        /*
        {
            GPC_ASSETS->AddShader("Leaves Compute", "particles.comp", Shader::COMPUTE);

            GPC_ASSETS->AddParticleProgram("Leaves Particle Program", {
                .Looping            = true,
                .MaxParticle        = 5000,
                .Mode               = ParticleEmissionMode::BURST,
                .Shape              = ParticleEmissionShape::RECTANGLE,
                .StartMinPosition   = glm::vec3(-1.0),
                .StartMaxPosition   = glm::vec3(1.0),
                .StartAfterSeconds  = 0.0f,
                .StartMinSpeed      = glm::vec3(-1.0),
                .StartMaxSpeed      = glm::vec3(1.0),
                .StartMinRotationSpeed = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                .StartMaxRotationSpeed = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                .StartMinSize       = glm::vec3(0.1),
                .StartMaxSize       = glm::vec3(0.5),
                .StartMinRotation   = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                .StartMaxRotation   = Transform::Euler_To_Quaternion(glm::pi<float>(), glm::pi<float>(), glm::pi<float>(), XYZ),
                .StartMaxColor      = Colors::ORANGE,
                .StartMinColor      = Colors::BLUE
                },
                GPC_SHADER("Leaves Compute")
            );
        }
        */

        GPC::MapGeneratorFactory::CreateFactory();

        mp_CaliFbxData = GPC_FBX->Load("Cali_Running", "Resources/Models/Characters/Cali_FullAnimation.fbx");
        mp_IrisFbxData = GPC_FBX->Load("Iris", "Resources/Models/Characters/Iris.fbx");
        mp_EnnemiFbxData = GPC_FBX->Load("Ennemi", "Resources/Models/Characters/Ennemi.fbx");
        GPC_FBX->Load("Boss", "Resources/Models/Characters/boss.fbx");

        // UI Loading
        {
            GPC_ASSETS->AddTexture("Background_Tooltip", "Resources/UI/VILLAGE/786x860/ui_tooltip.png");

            GPC_ASSETS->AddTextureArray(
                "786x860",
                786, 860,
                {
                    GPC_TEXTURE("Background_Tooltip"),
                }
            );

            GPC_ASSETS->AddTexture("Cali_Hud", "Resources/UI/HUD/626x213/ui_cali_hud_empty.png");
            GPC_ASSETS->AddTexture("Cali_Hp", "Resources/UI/HUD/626x213/ui_cali_hp.png");
            GPC_ASSETS->AddTextureArray(
                "626x213",
                626, 213,
                {
                    GPC_TEXTURE("Cali_Hud"),
                    GPC_TEXTURE("Cali_Hp"),
                }
            );

            GPC_ASSETS->AddTexture("Iris_Hud", "Resources/UI/HUD/300x139/ui_iris_hud_empty.png");
            GPC_ASSETS->AddTexture("Iris_Hp", "Resources/UI/HUD/300x139/ui_iris_hp.png");
            GPC_ASSETS->AddTextureArray(
                "300x139",
                300, 139,
                {
                    GPC_TEXTURE("Iris_Hud"),
                    GPC_TEXTURE("Iris_Hp"),
                }
            );

            GPC_ASSETS->AddTexture("Hud_Crystal", "Resources/UI/HUD/49x47/ui_hud_crystal.png");
            GPC_ASSETS->AddTexture("Hud_Crystal_T1", "Resources/UI/HUD/49x47/crystal/ui_hud_crystal_t1.png");
            GPC_ASSETS->AddTexture("Hud_Crystal_T2", "Resources/UI/HUD/49x47/crystal/ui_hud_crystal_t2.png");
            GPC_ASSETS->AddTexture("Hud_Crystal_T3", "Resources/UI/HUD/49x47/crystal/ui_hud_crystal_t3.png");
            GPC_ASSETS->AddTexture("Hud_Crystal_T4", "Resources/UI/HUD/49x47/crystal/ui_hud_crystal_t4.png");
            GPC_ASSETS->AddTexture("Hud_Crystal_T5", "Resources/UI/HUD/49x47/crystal/ui_hud_crystal_t5.png");
            GPC_ASSETS->AddTexture("Hud_Crystal_T6", "Resources/UI/HUD/49x47/crystal/ui_hud_crystal_t6.png");
            GPC_ASSETS->AddTexture("Hud_Crystal_Open", "Resources/UI/HUD/49x47/crystal/ui_hud_crystal_close.png");
            GPC_ASSETS->AddTexture("Hud_Crystal_Close", "Resources/UI/HUD/49x47/crystal/ui_hud_crystal_open.png");
            GPC_ASSETS->AddTexture("Hud_Fiber", "Resources/UI/HUD/49x47/ui_hud_fiber.png");
            GPC_ASSETS->AddTexture("Hud_Food", "Resources/UI/HUD/49x47/ui_hud_food.png");
            GPC_ASSETS->AddTexture("Hud_Ore", "Resources/UI/HUD/49x47/ui_hud_ore.png");
            GPC_ASSETS->AddTexture("Hud_Stone", "Resources/UI/HUD/49x47/ui_hud_stone.png");
            GPC_ASSETS->AddTexture("Hud_Wood", "Resources/UI/HUD/49x47/ui_hud_wood.png");
            GPC_ASSETS->AddTextureArray(
                "49x47",
                50, 67,
                {
                    GPC_TEXTURE("Hud_Crystal"),
                    GPC_TEXTURE("Hud_Crystal_T1"),
                    GPC_TEXTURE("Hud_Crystal_T2"),
                    GPC_TEXTURE("Hud_Crystal_T3"),
                    GPC_TEXTURE("Hud_Crystal_T4"),
                    GPC_TEXTURE("Hud_Crystal_T5"),
                    GPC_TEXTURE("Hud_Crystal_T6"),
                    GPC_TEXTURE("Hud_Crystal_Open"),
                    GPC_TEXTURE("Hud_Crystal_Close"),
                    GPC_TEXTURE("Hud_Fiber"),
                    GPC_TEXTURE("Hud_Food"),
                    GPC_TEXTURE("Hud_Ore"),
                    GPC_TEXTURE("Hud_Stone"),
                    GPC_TEXTURE("Hud_Wood"),
                }
            );

            //POSTGAME
            {
                GPC_ASSETS->AddTexture("PostGame_BG", "Resources/UI/POSTGAME/1920x871/ui_postgame.png");
                GPC_ASSETS->AddTextureArray("1920x871", 1920, 871,
            {
                    GPC_TEXTURE("PostGame_BG"),
                });

                GPC_ASSETS->AddTexture("ANIM00_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00000.png");
                GPC_ASSETS->AddTexture("ANIM01_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00001.png");
                GPC_ASSETS->AddTexture("ANIM02_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00002.png");
                GPC_ASSETS->AddTexture("ANIM03_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00003.png");
                GPC_ASSETS->AddTexture("ANIM04_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00004.png");
                GPC_ASSETS->AddTexture("ANIM05_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00005.png");
                GPC_ASSETS->AddTexture("ANIM06_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00006.png");
                GPC_ASSETS->AddTexture("ANIM07_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00007.png");
                GPC_ASSETS->AddTexture("ANIM08_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00008.png");
                GPC_ASSETS->AddTexture("ANIM09_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00009.png");
                GPC_ASSETS->AddTexture("ANIM10_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00010.png");
                GPC_ASSETS->AddTexture("ANIM11_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00011.png");
                GPC_ASSETS->AddTexture("ANIM12_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00012.png");
                GPC_ASSETS->AddTexture("ANIM13_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00013.png");
                GPC_ASSETS->AddTexture("ANIM14_PostGame_BG", "Resources/UI/POSTGAME/1920x871/ANIM/ui_postgame/ui_postgame_00014.png");

                GPC_ASSETS->AddTextureArray(
                    "ANIMATION_PostGame_BG",
                    1920, 871,
                    {
                        GPC_TEXTURE("ANIM00_PostGame_BG"),
                        GPC_TEXTURE("ANIM01_PostGame_BG"),
                        GPC_TEXTURE("ANIM02_PostGame_BG"),
                        GPC_TEXTURE("ANIM03_PostGame_BG"),
                        GPC_TEXTURE("ANIM04_PostGame_BG"),
                        GPC_TEXTURE("ANIM05_PostGame_BG"),
                        GPC_TEXTURE("ANIM06_PostGame_BG"),
                        GPC_TEXTURE("ANIM07_PostGame_BG"),
                        GPC_TEXTURE("ANIM08_PostGame_BG"),
                        GPC_TEXTURE("ANIM09_PostGame_BG"),
                        GPC_TEXTURE("ANIM10_PostGame_BG"),
                        GPC_TEXTURE("ANIM11_PostGame_BG"),
                        GPC_TEXTURE("ANIM12_PostGame_BG"),
                        GPC_TEXTURE("ANIM13_PostGame_BG"),
                        GPC_TEXTURE("ANIM14_PostGame_BG"),
                    }
                );

                GPC_ASSETS->AddTexture("PostGame_Fiber",   "Resources/UI/POSTGAME/95x93/ui_postgame_fiber.png");
                GPC_ASSETS->AddTexture("PostGame_Wood",    "Resources/UI/POSTGAME/95x93/ui_postgame_wood.png");
                GPC_ASSETS->AddTexture("PostGame_Food",    "Resources/UI/POSTGAME/95x93/ui_postgame_food.png");
                GPC_ASSETS->AddTexture("PostGame_Stone",   "Resources/UI/POSTGAME/95x93/ui_postgame_stone.png");
                GPC_ASSETS->AddTexture("PostGame_Ore",     "Resources/UI/POSTGAME/95x93/ui_postgame_ore.png");
                GPC_ASSETS->AddTexture("PostGame_Crystal_T1", "Resources/UI/POSTGAME/95x93/ui_postgame_crystal_t1.png");
                GPC_ASSETS->AddTexture("PostGame_Crystal_T2", "Resources/UI/POSTGAME/95x93/ui_postgame_crystal_t2.png");
                GPC_ASSETS->AddTexture("PostGame_Crystal_T3", "Resources/UI/POSTGAME/95x93/ui_postgame_crystal_t3.png");
                GPC_ASSETS->AddTexture("PostGame_Crystal_T4", "Resources/UI/POSTGAME/95x93/ui_postgame_crystal_t4.png");
                GPC_ASSETS->AddTexture("PostGame_Crystal_T5", "Resources/UI/POSTGAME/95x93/ui_postgame_crystal_t5.png");
                GPC_ASSETS->AddTexture("PostGame_Crystal_T6", "Resources/UI/POSTGAME/95x93/ui_postgame_crystal_t6.png");
                GPC_ASSETS->AddTextureArray("95x93", 95, 93,
                {
                    GPC_TEXTURE("PostGame_Fiber"),
                    GPC_TEXTURE("PostGame_Wood"),
                    GPC_TEXTURE("PostGame_Food"),
                    GPC_TEXTURE("PostGame_Stone"),
                    GPC_TEXTURE("PostGame_Ore"),
                    GPC_TEXTURE("PostGame_Crystal_T1"),
                    GPC_TEXTURE("PostGame_Crystal_T2"),
                    GPC_TEXTURE("PostGame_Crystal_T3"),
                    GPC_TEXTURE("PostGame_Crystal_T4"),
                    GPC_TEXTURE("PostGame_Crystal_T5"),
                    GPC_TEXTURE("PostGame_Crystal_T6"),
                });

                GPC_ASSETS->AddTexture("PostGame_Btn1_Normal", "Resources/UI/POSTGAME/703x145/ui_button_postgame_1.png");
                GPC_ASSETS->AddTexture("PostGame_Btn2_Normal", "Resources/UI/POSTGAME/703x145/ui_button_postgame_2.png");
                GPC_ASSETS->AddTextureArray(
                    "PostGame_Buttons_Normal",
                    703, 145,
                    {
                    GPC_TEXTURE("PostGame_Btn1_Normal"),
                    GPC_TEXTURE("PostGame_Btn2_Normal")
                    }
                );
                GPC_ASSETS->AddTexture("PostGame_Btn1_Selected", "Resources/UI/POSTGAME/723x165/ui_button_postgame_selected_1.png");
                GPC_ASSETS->AddTexture("PostGame_Btn2_Selected", "Resources/UI/POSTGAME/723x165/ui_button_postgame_selected_2.png");
                GPC_ASSETS->AddTextureArray(
                    "PostGame_Buttons_Selected",
                    723, 165,
                    {
                    GPC_TEXTURE("PostGame_Btn1_Selected"),
                    GPC_TEXTURE("PostGame_Btn2_Selected")
                    }
                );
            }
        }

        return ErrorType::SUCCESS;
    }

    void SceneExpedition::OnAssetsDestroy() {

    }

    ErrorType SceneExpedition::OnCreate(SceneInformation &info) {
        SceneDefault::OnCreate(info);
        SettingsGame::Get().Load();

        SetCollisionGridScale({ 5.0f * GENERAL_SCALE_EXPEDITION, 5.0f * GENERAL_SCALE_EXPEDITION });
        SetCollisionGridOffset({ 0.5f, 0.5f });

        auto nextMusic = CreateEntityAs3D();
        auto mp_VillageIntro = AddComponent<AudioSource>(nextMusic);

        mp_VillageIntro->Path = "Resources/Audio/Music/Ruins_of_hestia_explore.wav";
        mp_VillageIntro->Resource = GPC_AUDIO("ROH_village_intro");
        mp_VillageIntro->TypeVoice = AudioType::Music;
        mp_VillageIntro->Volume = SettingsGame::Get().GetVolume(AudioType::Music) / 3.0f;
        mp_VillageIntro->Looping = false;
        mp_VillageIntro->Play();

        // CreateDebugOverlay();

        LoadAllRooms();

        SetupAudio();

        CreatePlayer();

        CreatePlayerCamera();
        CreateFreeCamera();
        CreateSunLighting();
        CreateWater();

        UseFollowCamera();

        mp_Inventory = new Inventory();
        mp_Inventory->Initialize(false);

        SetupUI();

        CreateMiniMap();

        auto e = CreateEntityAs3D();
        mp_TransiFeuillle = AddBehavior<TransiFeuillle>(e);

        for (auto prop : mp_Props) {
            prop = nullptr;
        }

        m_isSceneVillage = false;
        return ErrorType::SUCCESS;
    }

    void SceneExpedition::OnDestroy() {
        // SceneDefault::OnDestroy();
    }

    ErrorType SceneExpedition::OnStart(SceneChangeCtx &ctx) {
        SceneDefault::OnStart(ctx);

        SceneExpeditionStartCtx* pCtx = static_cast<SceneExpeditionStartCtx *>(ctx.pData);

        m_Clock.Update();
        m_StartTime = m_Clock.GetTime();
        mp_PlayerInteractionBehavior->m_StartTime = m_StartTime;

        mp_TransiFeuillle->LaunchOpenAnimation(m_StartTime, this);
        //     MakeCallablePtr(
        //         +[](Scene* pScene) {
        //             pScene->EnableUI();
        //         }
        //     ));
        // DisableUI();

        bool NoCtx = false;
        if (pCtx == nullptr) {
            NoCtx = true;
            m_Difficulty = 1;
        } else {
            m_Difficulty = pCtx->MissionDifficulty;
        }

        StartMission();

        if (mp_Inventory && m_ResourceAddedHandle == 0) {
            m_ResourceAddedHandle = mp_Inventory->OnResourceAdded.ConnectLambda(
                [this](ResourceID id, uint32_t /*amount*/) {
                    UpdateResourceReactive(id);
                });
        }

        mp_PlayerLivingBehavior->InitAsCali();

        if (NoCtx) return ErrorType::SUCCESS;

        if (pCtx->pHelmet != nullptr)
            mp_PlayerLivingBehavior->EquipHelmet(pCtx->pHelmet);
        if (pCtx->pGloves != nullptr)
            mp_PlayerLivingBehavior->EquipGloves(pCtx->pGloves);
        if (pCtx->pChestPlate != nullptr)
            mp_PlayerLivingBehavior->EquipChestPlate(pCtx->pChestPlate);
        if (pCtx->pLegs != nullptr)
            mp_PlayerLivingBehavior->EquipLegs(pCtx->pLegs);
        if (pCtx->pBoots != nullptr)
            mp_PlayerLivingBehavior->EquipBoots(pCtx->pBoots);

        if (pCtx->pMelee != nullptr)
            mp_PlayerLivingBehavior->EquipMelee(pCtx->pMelee);
        if (pCtx->pDistance != nullptr)
            mp_PlayerLivingBehavior->EquipDistance(pCtx->pDistance);

        mp_PlayerLivingBehavior->CurrentHP = mp_PlayerLivingBehavior->pMaxHealth->GetFinalValue();

        return ErrorType::SUCCESS;
    }

    ErrorType SceneExpedition::OnEnd() {
        SceneDefault::OnEnd();
        // TODO: Disconnect OnResourceAdded apres fix SIGTRAP UiSignal
        DestroyMission();
        Purge();
        return ErrorType::SUCCESS;
    }

    void SceneExpedition::UpdateResourceReactive(ResourceID id) {
        if (!mp_Inventory) return;

        auto sumWood = [this]() {
            return mp_Inventory->GetResourceCount<Wood<ResourceTier::T1>>()
                 + mp_Inventory->GetResourceCount<Wood<ResourceTier::T2>>()
                 + mp_Inventory->GetResourceCount<Wood<ResourceTier::T3>>()
                 + mp_Inventory->GetResourceCount<Wood<ResourceTier::T4>>()
                 + mp_Inventory->GetResourceCount<Wood<ResourceTier::T5>>();
        };
        auto sumStone = [this]() {
            return mp_Inventory->GetResourceCount<Stone<ResourceTier::T1>>()
                 + mp_Inventory->GetResourceCount<Stone<ResourceTier::T2>>()
                 + mp_Inventory->GetResourceCount<Stone<ResourceTier::T3>>()
                 + mp_Inventory->GetResourceCount<Stone<ResourceTier::T4>>()
                 + mp_Inventory->GetResourceCount<Stone<ResourceTier::T5>>();
        };
        auto sumMetal = [this]() {
            return mp_Inventory->GetResourceCount<Metal<ResourceTier::T1>>()
                 + mp_Inventory->GetResourceCount<Metal<ResourceTier::T2>>()
                 + mp_Inventory->GetResourceCount<Metal<ResourceTier::T3>>()
                 + mp_Inventory->GetResourceCount<Metal<ResourceTier::T4>>()
                 + mp_Inventory->GetResourceCount<Metal<ResourceTier::T5>>();
        };
        auto sumFiber = [this]() {
            return mp_Inventory->GetResourceCount<Fiber<ResourceTier::T1>>()
                 + mp_Inventory->GetResourceCount<Fiber<ResourceTier::T2>>()
                 + mp_Inventory->GetResourceCount<Fiber<ResourceTier::T3>>()
                 + mp_Inventory->GetResourceCount<Fiber<ResourceTier::T4>>()
                 + mp_Inventory->GetResourceCount<Fiber<ResourceTier::T5>>();
        };
        auto sumGemmes = [this]() {
            return mp_Inventory->GetResourceCount<PurpleGemme>()
                 + mp_Inventory->GetResourceCount<RedGemme>()
                 + mp_Inventory->GetResourceCount<BlueGemme>()
                 + mp_Inventory->GetResourceCount<GreenGemme>()
                 + mp_Inventory->GetResourceCount<YellowGemme>()
                 + mp_Inventory->GetResourceCount<OrangeGemme>();
        };

        if (id == Wood<ResourceTier::T1>::ResourceId
         || id == Wood<ResourceTier::T2>::ResourceId
         || id == Wood<ResourceTier::T3>::ResourceId
         || id == Wood<ResourceTier::T4>::ResourceId
         || id == Wood<ResourceTier::T5>::ResourceId) {
            m_WoodReactive.Set(std::to_string(sumWood()));
        }
        else if (id == Stone<ResourceTier::T1>::ResourceId
              || id == Stone<ResourceTier::T2>::ResourceId
              || id == Stone<ResourceTier::T3>::ResourceId
              || id == Stone<ResourceTier::T4>::ResourceId
              || id == Stone<ResourceTier::T5>::ResourceId) {
            m_StoneReactive.Set(std::to_string(sumStone()));
        }
        else if (id == Metal<ResourceTier::T1>::ResourceId
              || id == Metal<ResourceTier::T2>::ResourceId
              || id == Metal<ResourceTier::T3>::ResourceId
              || id == Metal<ResourceTier::T4>::ResourceId
              || id == Metal<ResourceTier::T5>::ResourceId) {
            m_OreReactive.Set(std::to_string(sumMetal()));
        }
        else if (id == Fiber<ResourceTier::T1>::ResourceId
              || id == Fiber<ResourceTier::T2>::ResourceId
              || id == Fiber<ResourceTier::T3>::ResourceId
              || id == Fiber<ResourceTier::T4>::ResourceId
              || id == Fiber<ResourceTier::T5>::ResourceId) {
            m_FiberReactive.Set(std::to_string(sumFiber()));
        }
        else if (id == Food::ResourceId) {
            m_FoodReactive.Set(std::to_string(mp_Inventory->GetResourceCount<Food>()));
        }
        else if (id == PurpleGemme::ResourceId) {
            m_CrystalT1Reactive.Set(std::to_string(mp_Inventory->GetResourceCount<PurpleGemme>()));
            m_CrystalReactive.Set(std::to_string(sumGemmes()));
        }
        else if (id == RedGemme::ResourceId) {
            m_CrystalT2Reactive.Set(std::to_string(mp_Inventory->GetResourceCount<RedGemme>()));
            m_CrystalReactive.Set(std::to_string(sumGemmes()));
        }
        else if (id == BlueGemme::ResourceId) {
            m_CrystalT3Reactive.Set(std::to_string(mp_Inventory->GetResourceCount<BlueGemme>()));
            m_CrystalReactive.Set(std::to_string(sumGemmes()));
        }
        else if (id == GreenGemme::ResourceId) {
            m_CrystalT4Reactive.Set(std::to_string(mp_Inventory->GetResourceCount<GreenGemme>()));
            m_CrystalReactive.Set(std::to_string(sumGemmes()));
        }
        else if (id == YellowGemme::ResourceId) {
            m_CrystalT5Reactive.Set(std::to_string(mp_Inventory->GetResourceCount<YellowGemme>()));
            m_CrystalReactive.Set(std::to_string(sumGemmes()));
        }
        else if (id == OrangeGemme::ResourceId) {
            m_CrystalT6Reactive.Set(std::to_string(mp_Inventory->GetResourceCount<OrangeGemme>()));
            m_CrystalReactive.Set(std::to_string(sumGemmes()));
        }
    }

    void SceneExpedition::OnSceneUpdate() {
        SceneDefault::OnSceneUpdate();

        if (mp_PlayerLivingBehavior && mp_PlayerLivingBehavior->pMaxHealth)
        {
            const int curHp = static_cast<int>(mp_PlayerLivingBehavior->CurrentHP + 0.5f);
            const int maxHp = static_cast<int>(mp_PlayerLivingBehavior->pMaxHealth->GetFinalValue() + 0.5f);
            m_PlayerHpReactive.Set(std::to_string(curHp) + "/" + std::to_string(maxHp));
        }

        if (m_MovingTutoHasProc == false && m_Clock.GetTime() > m_StartTime + 3.0f
            && TutoQueue::CanSpawnTuto(static_cast<float>(m_Clock.GetTime()))) {
            if (m_UseTuto) {
                mp_UIpopSfx->Play();
                auto tuto1 = CreateEntityAs3D();
                AddBehavior<ExpeditionMovementTuto>(tuto1);
                m_MovingTutoHasProc = true;
            }
        }

        if (UiAnim::IsAnimating(*this, mp_PlayerMiniMapIndicator) == false) {
            glm::vec2 position_in_map = {
                mp_PlayerTransform->GetWorldPosition().x / (static_cast<float>(MapGeneratorData::ROOM_SCALE) * static_cast<float>(MAX_MAP_SIZE)),
                mp_PlayerTransform->GetWorldPosition().z / (static_cast<float>(MapGeneratorData::ROOM_SCALE) * static_cast<float>(MAX_MAP_SIZE))
            };
            position_in_map = 1.0f - position_in_map;

            constexpr float MINIMAP_INDICATOR_RANGE = 180.f;
            constexpr float MINIMAP_INDICATOR_HALF  = MINIMAP_INDICATOR_RANGE * 0.5f;
            glm::vec2 mapPos = position_in_map * MINIMAP_INDICATOR_RANGE;
            if (m_MiniMapRotation != 0.0f) {
                const glm::vec2 centered = mapPos - glm::vec2(MINIMAP_INDICATOR_HALF);
                const float c = std::cos(m_MiniMapRotation);
                const float s = std::sin(m_MiniMapRotation);
                mapPos = glm::vec2(centered.x * c - centered.y * s,
                                   centered.x * s + centered.y * c)
                       + glm::vec2(MINIMAP_INDICATOR_HALF);
            }
            UiAnim::MoveTo(*this, mp_PlayerMiniMapIndicator, mapPos, 0.0f, Tweening::EasingType::Linear);
        }

        auto& program = GPC_GRAPHIC_PROGRAM("Map Program")->Program;
        program.Update(program.GetBindingSlot("UniformRoomTileTextureID"), &m_UniformTextureIDs, sizeof(UniformTextureID), 0);

        if (Inputs::IsKeyPress(Inputs::KeyCode::F1)) {
            UseFollowCamera();
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::F2)) {
            UseFreeCamera();
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::F3)) {
            SetPlayerAtEnd();
        }

        if (Inputs::IsKeyPress(Inputs::KeyCode::F4)) {
            m_Clock.SetTimeScale(0.0);
        }
        if (Inputs::IsKeyPress(Inputs::KeyCode::F5)) {
            m_Clock.SetTimeScale(1.0);
        }
        // if (Inputs::IsKeyPress(Inputs::KeyCode::F6)) {
        //     SetPlayerAtEnd();
        //     mp_PlayerTransform->LocalTransform.SetPosition(
        //         mp_PlayerTransform->LocalTransform.GetPosition() + glm::vec3(0, 50, 0)
        //     );
        // }

    }

} // GPC