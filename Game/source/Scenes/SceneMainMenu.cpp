#include "Scenes/SceneMainMenu.h"

#include "SettingsGame.h"
#include "UI/UiBuilder.h"
#include "AudioSystem.h"
#include "SerialFile.h"
#include "SerialTree.h"
#include "UI/UiAnim.h"
#include "UI/UiAnimFluent.h"
#include "Dialogue/DialogueBehaviour.h"
#include "Village/Save.h"

namespace GPC
{
    struct DialogueBehaviour;

    ErrorType SceneMainMenu::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();
        LoadUIElements();

        return ErrorType::SUCCESS;
    }

    ErrorType SceneMainMenu::OnCreate(SceneInformation& info)
    {
        SceneDefault::OnCreate(info);
        SettingsGame::Get().Load();

        SerialTree tree;
        if (SerialFile::Load(Saves::SAVE_FILE_PATH, tree) == ErrorType::SUCCESS)
        {
            m_HasSaved = true;
        }

        auto settingsEntity = CreateEntityAs2D();
        mp_SettingsBehaviour = AddBehavior<UISettings>(settingsEntity);

        SetupUI();
        SetupAudio();

        return ErrorType::SUCCESS;
    }

    ErrorType SceneMainMenu::OnStart(SceneChangeCtx& ctx)
    {
        m_SceneCtx = ctx;

        return SceneDefault::OnStart(ctx);
    }

    void SceneMainMenu::OnSceneUpdate()
    {
        SceneDefault::OnSceneUpdate();

        if (mp_DialogueBehaviour && mp_DialogueBehaviour->IsDialogueEnded())
        {
            constexpr CtxMainMenuToVillage c = { false };

            SceneChangeCtx ctx{};
            memcpy(&ctx.pData, &c, sizeof(CtxMainMenuToVillage));
            ctx.DataSize = sizeof(CtxMainMenuToVillage);

            SwapToScene("Village", ctx);
        }

    }

    void SceneMainMenu::OnDestroy()
    {

    }

    void SceneMainMenu::SetupUI()
    {
        UiBuilder builder(this);

        UiCanvas* rootUI = builder
            .At(Anchors::TOP_LEFT, {0, 0})
            .Tint(Colors::TRANSPARENT)
            .FitToScreen(true)
            .BuildCanvas();

        const EntityID rootUiID = rootUI->GetEntityID();

        builder
            .At(Anchors::MIDDLE_MIDDLE, {0, 0})
            .Scale({1.0f, 1.0f})
            .Sprite("keyart", "MainMenu_Background")
            .ChildOf(rootUiID)
            .BuildSprite();

        mp_GameBlurUi = builder.At(Anchors::TOP_MIDDLE, {0,20})
            .Scale({0.2f,0.2f})
            .Sprite("MainMenu","MainMenu_Title_blur")
            .Tint(Color(100, 100, 100, 200))
            .ChildOf(rootUiID)
            .BuildSprite();

        mp_GameNameUi = builder
            .At(Anchors::TOP_MIDDLE, {0, 20})
            .Scale({0.7f, 0.7f})
            .Sprite("MainMenu_Title_Array", "MainMenu_Title")
            .ChildOf(rootUiID)
            .BuildSprite();

        mp_LogoStudio = builder
                        .At(Anchors::BOTTOM_LEFT, {4, -4})
                        .Scale({0.2f,0.2f})
                        .Sprite("619x642", "LogoStudio")
                        .ChildOf(rootUiID)
                        .Tint(Colors::WHITE)
                        .BuildSprite();

        mp_LogoStudio->DrawOrder = 10000;

        SetupMenuBoutons(builder, rootUiID);
    }

    void SceneMainMenu::SetupMenuBoutons(UiBuilder& builder, EntityID parent)
    {
        constexpr float BUTTON_WIDTH = 280.f;
        constexpr float BUTTON_HEIGHT = 50.f;
        constexpr float BUTTON_GAP = 20.f;
        constexpr float START_OFFSET_Y = 40.f;
        const Color BUTTON_NORMAL = Color(20, 20, 20, 180);

        mp_MenuUI = builder
            .At(Anchors::MIDDLE_MIDDLE, {0, 0})
            .Size({1080, 720})
            .Tint(Colors::TRANSPARENT)
            .ChildOf(parent)
            .BuildCanvas();

        const EntityID menuPanelID = mp_MenuUI->GetEntityID();

        auto createButton = [&](int index, const std::string& label, Callable<void>* action, const bool isActivated = true) -> UiButton*
        {
            float posY = START_OFFSET_Y + index * (BUTTON_HEIGHT + BUTTON_GAP);

            UiSprite* btnSprite = builder.ChildOf(menuPanelID)
                     .At(Anchors::MIDDLE_MIDDLE, { 0.0f, (int)posY })
                     .Scale({0.7f,0.7f})
                     .Sprite("Main_Menu_Buttons", "ButtonUnSelected")
                     .BuildSprite();
            if (!btnSprite)
            {
                return nullptr;
            }

            const EntityID btnID = btnSprite->GetEntityID();
            UiButton* btn = builder.BuildButton(btnSprite);

            auto text = builder
                .At(Anchors::MIDDLE_MIDDLE, {0, 5})
                .Font(UiTheme::DefaultFont)
                .FontSize(22)
                .Tint(Colors::PERFECT_BROWN)
                .Text(label)
                .ChildOf(btnID)
                .BuildText();


            if (!isActivated)
            {
                btnSprite->SetTint(Colors::DARK_GRAY);
                text->SetTint(Colors::DARK_GRAY);
            }
            else
            {
                UiAnim::On(*this, btnSprite)
                     .FadeTo(1.0f, 5.0f)
                     .OnUpdate([btnSprite, btn](float t) {
                         if (btn->IsHovered && !btn->IsDisabled)
                         {
                             int frameNum = static_cast<int>(t * 14.9f);
                             std::string frameName = "ANIM";
                             if (frameNum < 10) frameName += "0";
                             frameName += std::to_string(frameNum) + "_Button";

                             btnSprite->SetSprite("ANIMATION_BUTTON", frameName);
                         }
                     })
                     .Repeat(-1)
                     .Play();

                btn->OnHover.ConnectLambda([this, btnSprite]() {
                     UiAnim::ScaleTo(*this, btnSprite, {0.75f, 0.75f}, 0.15f);
                 });

                btn->OnLeave.ConnectLambda([this, btnSprite]() {
                    btnSprite->SetSprite("Main_Menu_Buttons", "ButtonUnSelected");
                    UiAnim::ScaleTo(*this, btnSprite, {0.7f, 0.7f}, 0.15f);
                });

                if (btn && action) {
                    btn->OnClick.Connect(action);
                }
            }

            return btn;
        };

        mp_ContinueButton = createButton(0, "Continuer", MakeCallablePtr([this]
        {
            if (m_HasSaved)
            {
               const CtxMainMenuToVillage c = { true };

               SceneChangeCtx ctx{};
               memcpy(&ctx, &c, sizeof(CtxMainMenuToVillage));
               ctx.DataSize = sizeof(CtxMainMenuToVillage);

               SwapToScene("Village", ctx);
            }
        }), m_HasSaved);

        if (m_HasSaved)
        {
            mp_ContinueButton->OnClick.ConnectLambda([this]() { if (mp_UIpoOpen) mp_UIpoOpen->Play(); });
            mp_ContinueButton->OnHover.ConnectLambda([this]()
            {
                if (mp_UIHover) mp_UIHover->Play();
            });
        }

        mp_NewGameButton = createButton(1, "Nouvelle Partie", MakeCallablePtr([this]
        {
            NewGame();
        }));
        mp_NewGameButton->OnClick.ConnectLambda([this]() { if (mp_UIpoOpen) mp_UIpoOpen->Play(); });
        mp_NewGameButton->OnHover.ConnectLambda([this]() { if (mp_UIHover) mp_UIHover->Play(); });

        mp_SettingsButton = createButton(2, "Paramètres", MakeCallablePtr([this]
        {
            settingsIsVisible = !settingsIsVisible;
            if (mp_SettingsBehaviour) mp_SettingsBehaviour->Toggle(settingsIsVisible);
            if (mp_MenuUI) mp_MenuUI->IsEnable = !settingsIsVisible;
            DisabledAllButtons(settingsIsVisible);
        }));
        mp_SettingsButton->OnClick.ConnectLambda([this]() { if (mp_UIpoOpen) mp_UIpoOpen->Play(); });
        mp_SettingsButton->OnHover.ConnectLambda([this]() { if (mp_UIHover) mp_UIHover->Play(); });

        mp_SettingsBehaviour->OnSettingsChanged.ConnectLambda([this]()
        {
            mp_MainMusic->Volume = SettingsGame::Get().GetVolume(AudioType::Music);
            UpdateSFXVolume();
            SettingsGame::Get().Save();
        });

        mp_SettingsBehaviour->OnCloseRequested.ConnectLambda([this]() {
            this->settingsIsVisible = false;
            if (this->mp_MenuUI) this->mp_MenuUI->IsEnable = true;
            this->DisabledAllButtons(false);
            mp_UIpopClose->Play();
        });
    }

    void SceneMainMenu::DisabledAllButtons(bool next)
    {
        if (mp_ContinueButton) mp_ContinueButton->IsDisabled = next;
        if (mp_NewGameButton) mp_NewGameButton->IsDisabled = next;
        if (mp_SettingsButton) mp_SettingsButton->IsDisabled = next;
    }

    void SceneMainMenu::UpdateSFXVolume()
    {
        mp_UIpoOpen->Volume = SettingsGame::Get().GetVolume(AudioType::SFX);
        mp_UIpopClose->Volume = SettingsGame::Get().GetVolume(AudioType::SFX);
        mp_UIHover->Volume = SettingsGame::Get().GetVolume(AudioType::SFX);
    }

    void SceneMainMenu::SetupAudio()
    {
        EntityID MainMusicID = CreateEntityAs3D();
        mp_MainMusic = AddComponent<AudioSource>(MainMusicID);
        mp_MainMusic->Path = "Resources/Audio/Music/Ecran_titre.wav";
        mp_MainMusic->Resource = GPC_AUDIO("Ecran_titre");
        mp_MainMusic->TypeVoice = AudioType::Music;
        mp_MainMusic->Volume = SettingsGame::Get().GetVolume(AudioType::Music);
        mp_MainMusic->Looping = true;
        mp_MainMusic->Play();

        mp_UIpoOpen = AddComponent<AudioSource>(CreateEntityAs2D());
        mp_UIpoOpen->Path = "Resources/Audio/SFX/sfx_ui1.wav";
        mp_UIpoOpen->Resource = GPC_AUDIO("UI1");
        mp_UIpoOpen->TypeVoice = AudioType::SFX;
        mp_UIpoOpen->Volume = SettingsGame::Get().GetVolume(AudioType::SFX);
        mp_UIpoOpen->Looping = false;

        mp_UIpopClose = AddComponent<AudioSource>(CreateEntityAs2D());
        mp_UIpopClose->Path = "Resources/Audio/SFX/sfx_ui2.wav";
        mp_UIpopClose->Resource = GPC_AUDIO("UI2");
        mp_UIpopClose->TypeVoice = AudioType::SFX;
        mp_UIpopClose->Volume = SettingsGame::Get().GetVolume(AudioType::SFX);
        mp_UIpopClose->Looping = false;

        mp_UIHover = AddComponent<AudioSource>(CreateEntityAs2D());
        mp_UIHover->Path = "Resources/Audio/SFX/sfx_hoover.wav";
        mp_UIHover->Resource = GPC_AUDIO("Hover");
        mp_UIHover->TypeVoice = AudioType::SFX;
        mp_UIHover->Volume = 1.0f;
        mp_UIHover->Looping = false;
    }

    void SceneMainMenu::LoadUIElements()
    {
        GPC_ASSETS->AddTexture("MainMenu_Background", "Resources/UI/MAIN_MENU/1920x1080/keyart.png");
        GPC_ASSETS->AddTextureArray(
            "keyart",
            1920, 1080,
            {
                GPC_TEXTURE("MainMenu_Background")
            }
        );

        GPC_ASSETS->AddTexture("MainMenu_Title_blur", "Resources/UI/MAIN_MENU/3416x2160/blur bg.png");
        GPC_ASSETS->AddTextureArray(
            "MainMenu",
            3416, 2160,
            {
            GPC_TEXTURE("MainMenu_Title_blur")
            }
        );

        GPC_ASSETS->AddTexture("MainMenu_Title", "Resources/UI/MAIN_MENU/1102x673/ui_title.png");
        GPC_ASSETS->AddTextureArray(
            "MainMenu_Title_Array",
            1102, 673,
            {
                GPC_TEXTURE("MainMenu_Title")
            }
        );

        GPC_ASSETS->AddTexture("ButtonUnSelected", "Resources/UI/MAIN_MENU/411x99/ui_button_title.png");
        GPC_ASSETS->AddTexture("ButtonSelected", "Resources/UI/MAIN_MENU/411x99/ui_button_title_selected.png");
        GPC_ASSETS->AddTextureArray(
            "Main_Menu_Buttons",
            411, 99,
            {
            GPC_TEXTURE("ButtonUnSelected"),
            GPC_TEXTURE("ButtonSelected")
            }
        );

        GPC_ASSETS->AddTexture("ANIM00_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00000.png");
        GPC_ASSETS->AddTexture("ANIM01_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00001.png");
        GPC_ASSETS->AddTexture("ANIM02_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00002.png");
        GPC_ASSETS->AddTexture("ANIM03_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00003.png");
        GPC_ASSETS->AddTexture("ANIM04_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00004.png");
        GPC_ASSETS->AddTexture("ANIM05_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00005.png");
        GPC_ASSETS->AddTexture("ANIM06_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00006.png");
        GPC_ASSETS->AddTexture("ANIM07_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00007.png");
        GPC_ASSETS->AddTexture("ANIM08_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00008.png");
        GPC_ASSETS->AddTexture("ANIM09_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00009.png");
        GPC_ASSETS->AddTexture("ANIM10_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00010.png");
        GPC_ASSETS->AddTexture("ANIM11_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00011.png");
        GPC_ASSETS->AddTexture("ANIM12_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00012.png");
        GPC_ASSETS->AddTexture("ANIM13_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00013.png");
        GPC_ASSETS->AddTexture("ANIM14_Button", "Resources/UI/MAIN_MENU/411x99/ANIM/ui_button_title_selected/ui_button_title_selected_00014.png");

        GPC_ASSETS->AddTextureArray(
            "ANIMATION_BUTTON",
            411, 99,
            {
                GPC_TEXTURE("ANIM00_Button"),
                GPC_TEXTURE("ANIM01_Button"),
                GPC_TEXTURE("ANIM02_Button"),
                GPC_TEXTURE("ANIM03_Button"),
                GPC_TEXTURE("ANIM04_Button"),
                GPC_TEXTURE("ANIM05_Button"),
                GPC_TEXTURE("ANIM06_Button"),
                GPC_TEXTURE("ANIM07_Button"),
                GPC_TEXTURE("ANIM08_Button"),
                GPC_TEXTURE("ANIM09_Button"),
                GPC_TEXTURE("ANIM10_Button"),
                GPC_TEXTURE("ANIM11_Button"),
                GPC_TEXTURE("ANIM12_Button"),
                GPC_TEXTURE("ANIM13_Button"),
                GPC_TEXTURE("ANIM14_Button"),
            }
        );

        GPC_ASSETS->AddTexture("LogoStudio", "Resources/Images/LogoStudio.png");
        GPC_ASSETS->AddTextureArray(
            "LogoStudio",
            619, 642,
            {

            }
        );
    }

    void SceneMainMenu::NewGame()
    {
        mp_MenuUI->IsEnable = false;
        DisabledAllButtons(true);
        std::remove(Saves::SAVE_FILE_PATH.c_str());

        UiAnim::On(*this, mp_GameNameUi)
        .FadeOut(2.0f)
        .OnComplete([this, elt = mp_GameNameUi]
        {
            elt->IsEnable = false;
            this->StartPrologue();
        })
        .Play();

        UiAnim::On(*this, mp_GameBlurUi)
        .FadeOut(2.0f)
        .Play();

        UiAnim::On(*this, mp_LogoStudio)
        .FadeOut(2.0f)
        .Play();

    }

    void SceneMainMenu::StartPrologue()
    {
        StartPrologueDialogue();
    }

    void SceneMainMenu::StartPrologueDialogue()
    {
        auto dialogue = CreateEntityAs2D();
        mp_DialogueBehaviour = AddBehavior<DialogueBehaviour>(dialogue);


        mp_DialogueBehaviour->StartDialogue("Resources/DialoguesText/Prologue/Prologue.txt", std::vector<UiCanvas*>());
    }
}
