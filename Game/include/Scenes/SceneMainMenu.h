//
// Created by wgsdp on 22/04/2026.
//

#ifndef RUINS_OF_HESTIA_SCENEMAINMENU_H
#define RUINS_OF_HESTIA_SCENEMAINMENU_H

#include "SceneDefault.h"
#include "UISettingsGame_Behavior.h"
#include "UI/UiBuilder.h"

struct CtxMainMenuToVillage
{
    bool isContinue = false;
};

namespace GPC
{
    struct DialogueBehaviour;
    class AudioSource;
    class SceneMainMenu : public SceneDefault
    {
    public:
        ErrorType OnAssetsLoad() override;
        ErrorType OnCreate(SceneInformation& info) override;
        ErrorType OnStart(SceneChangeCtx& ctx) override;
        void OnSceneUpdate() override;
        void OnDestroy() override;

    private:
        UISettings* mp_SettingsBehaviour = nullptr;
        bool settingsIsVisible = false;

        UiSprite* mp_LogoStudio = nullptr;
        UiCanvas* mp_SettingsUI = nullptr;
        UiCanvas* mp_MenuUI = nullptr;
        UiSprite* mp_GameNameUi = nullptr;
        UiSprite* mp_GameBlurUi = nullptr;
        UiButton* mp_ContinueButton = nullptr;
        UiButton* mp_LoadButton = nullptr;
        UiButton* mp_NewGameButton = nullptr;
        UiButton* mp_SettingsButton = nullptr;

        AudioSource* mp_MainMusic = nullptr;
        AudioSource* mp_UIpoOpen = nullptr;
        AudioSource* mp_UIpopClose = nullptr;
        AudioSource* mp_UIHover = nullptr;

        DialogueBehaviour* mp_DialogueBehaviour = nullptr;

        bool m_HasSaved = false;

        SceneChangeCtx m_SceneCtx;

    private:
        void SetupUI();
        void SetupMenuBoutons(UiBuilder& builder, EntityID parent);
        void SetupAudio();

        void DisabledAllButtons(bool next);
        void UpdateSFXVolume();

        void LoadUIElements();

        void NewGame();
        void StartPrologue();
        void StartPrologueDialogue();
    };
}

#endif //RUINS_OF_HESTIA_SCENEMAINMENU_H