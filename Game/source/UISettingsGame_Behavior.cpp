#include "UISettingsGame_Behavior.h"

#include "AnimationComponent.h"
#include "Scene.h"
#include "SettingsGame.h"

namespace GPC
{
    void UISettings::OnCreate(const BehaviorCreateContext* pCtx)
    {
        Behavior::OnCreate(pCtx);
    }

    void UISettings::OnStart(const BehaviorCreateContext* pCtx)
    {
        Behavior::OnStart(pCtx);
        SettingsGame::Get().Load();

        auto& audio = SettingsGame::Get().GetAudio();
        auto& video = SettingsGame::Get().GetVideo();

        m_MasterVolume.Set(static_cast<int>(audio.MasterVolume * 100.0f));
        m_MusicVolume.Set(static_cast<int>(audio.MusicVolume * 100.0f));
        m_SfxVolume.Set(static_cast<int>(audio.SFXVolume * 100.0f));

        m_FpsValue.Set(video.FPS);

        mp_Scene = pCtx->pScene;
        UiBuilder b(mp_Scene);

        mp_SettingsCanvas = b.At(Anchors::MIDDLE_MIDDLE, {0, 0})
                            .Size({ 1080.0f, 720.0f })
                            .Tint(Colors::TRANSPARENT)
                            .BuildCanvas();

        if (mp_SettingsCanvas) {
            mp_SettingsCanvas->IsEnable = false;
        }

        mp_SettingsSprite = b.ChildOf(mp_SettingsCanvas->GetEntityID())
                            .At(Anchors::MIDDLE_MIDDLE, { 0, 0 })
                            .Scale({0.6f, 0.5f})
                            .Sprite("1443x998", "Settings_Background")
                            .BuildSprite();

        UiSprite* closeButton = b.ChildOf(mp_SettingsCanvas->GetEntityID())
                                .At(Anchors::TOP_MIDDLE, { 400, 90})
                                .Scale({0.8f, 0.8f})
                                .Sprite("218x169", "Button_Delete")
                                .BuildSprite();
        if (!closeButton) return;

        mp_SettingsCloseButton = b.BuildButton(closeButton);
        if (mp_SettingsCloseButton) {
            mp_SettingsCloseButton->OnClick.ConnectLambda([this]() {
                this->Toggle(false);
                this->OnCloseRequested.Emit();
            });
        }

        BuildVolumeUI(b);
        BuildGraphicsUI(b);
    }

    void UISettings::OnUpdate(const BehaviorUpdateContext* pCtx)
    {
        Behavior::OnUpdate(pCtx);
    }

    void UISettings::BuildVolumeUI(UiBuilder& builder)
    {
         const EntityID settingsId = mp_SettingsSprite->GetEntityID();

        float startY = 100.0f;
        float startX = 150.0f;
        float offsetMinus = 550.0f;
        float offsetPlus = 660.0f;
        float offsetNumber = 605.0f;

        builder.ChildOf(settingsId).At(Anchors::TOP_MIDDLE, { 0, 50 }).FontSize(50.0f).Tint(Colors::BLACK).Text("Paramètres").BuildText();

        builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { startX, startY })
               .FontSize(30.0f).Tint(Colors::BLACK).Text("Volume Principale").BuildText();

        UiSprite* mMinusS = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetMinus, startY}).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_Minus").BuildSprite();
        UiButton* mMinusB = builder.BuildButton(mMinusS);

        UiText* mVal = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetNumber, startY + 5.0f})
                              .FontSize(30.0f).Tint(Colors::BLACK).Text("0").BuildText();
        mVal->BindReactive(m_MasterVolume);

        UiSprite* mPlusS = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetPlus, startY }).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_Plus").BuildSprite();
        UiButton* mPlusB = builder.BuildButton(mPlusS);

        if (mMinusB) {
            mMinusB->OnClick.ConnectLambda([this]() {
                float v = std::clamp(SettingsGame::Get().GetAudio().MasterVolume - 0.1f, 0.0f, 1.0f);
                SettingsGame::Get().SetMasterVolume(v);
                this->m_MasterVolume.Set(static_cast<int>(v * 100.0f));
                this->OnSettingsChanged.Emit();
            });
        }
        if (mPlusB) {
            mPlusB->OnClick.ConnectLambda([this]() {
                float v = std::clamp(SettingsGame::Get().GetAudio().MasterVolume + 0.1f, 0.0f, 1.0f);
                SettingsGame::Get().SetMasterVolume(v);
                this->m_MasterVolume.Set(static_cast<int>(v * 100.0f));
                this->OnSettingsChanged.Emit();
            });
        }

        float musicY = startY + 50.0f;
        builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { startX, musicY })
               .FontSize(30.0f).Tint(Colors::BLACK).Text("Volume de la musique").BuildText();

        UiSprite* muMinusS = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetMinus, musicY }).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_Minus").BuildSprite();
        UiButton* muMinusB = builder.BuildButton(muMinusS);

        UiText* muVal = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetNumber, musicY + 5.0f }).FontSize(30.0f).Tint(Colors::BLACK).Text("0").BuildText();
        muVal->BindReactive(m_MusicVolume);

        UiSprite* muPlusS = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetPlus, musicY}).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_Plus").BuildSprite();
        UiButton* muPlusB = builder.BuildButton(muPlusS);

        if (muMinusB) {
            muMinusB->OnClick.ConnectLambda([this]() {
                float v = std::clamp(SettingsGame::Get().GetAudio().MusicVolume - 0.1f, 0.0f, 1.0f);
                SettingsGame::Get().SetMusicVolume(v);
                this->m_MusicVolume.Set(static_cast<int>(v * 100.0f));
                this->OnSettingsChanged.Emit();
            });
        }
        if (muPlusB) {
            muPlusB->OnClick.ConnectLambda([this]() {
                float v = std::clamp(SettingsGame::Get().GetAudio().MusicVolume + 0.1f, 0.0f, 1.0f);
                SettingsGame::Get().SetMusicVolume(v);
                this->m_MusicVolume.Set(static_cast<int>(v * 100.0f));
                this->OnSettingsChanged.Emit();
            });
        }

        float sfxY = musicY + 50.0f;
        builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { startX, sfxY })
               .FontSize(30.0f).Tint(Colors::BLACK).Text("Volume des SFX").BuildText();

        UiSprite* sfxMinusS = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetMinus, sfxY }).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_Minus").BuildSprite();
        UiButton* sfxMinusB = builder.BuildButton(sfxMinusS);

        UiText* sfxVal = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetNumber, sfxY + 5.0f }).FontSize(30.0f).Tint(Colors::BLACK).Text("0").BuildText();
        sfxVal->BindReactive(m_SfxVolume);

        UiSprite* sfxPlusS = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetPlus, sfxY }).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_Plus").BuildSprite();
        UiButton* sfxPlusB = builder.BuildButton(sfxPlusS);

        if (sfxMinusB) {
            sfxMinusB->OnClick.ConnectLambda([this]() {
                float current = SettingsGame::Get().GetAudio().SFXVolume;
                float v = std::clamp(current - 0.1f, 0.0f, 1.0f);
                SettingsGame::Get().SetSFXVolume(v);
                this->m_SfxVolume.Set(static_cast<int>(v * 100.0f));
                this->OnSettingsChanged.Emit();
            });
        }

        if (sfxPlusB) {
            sfxPlusB->OnClick.ConnectLambda([this]() {
                float current = SettingsGame::Get().GetAudio().SFXVolume;
                float v = std::clamp(current + 0.1f, 0.0f, 1.0f);
                SettingsGame::Get().SetSFXVolume(v);
                this->m_SfxVolume.Set(static_cast<int>(v * 100.0f));
                this->OnSettingsChanged.Emit();
            });
        }
    }

    void UISettings::BuildGraphicsUI(UiBuilder& builder)
    {
        const EntityID settingsId = mp_SettingsSprite->GetEntityID();

        float screenY = 300.0f;
        float startX = 150.0f;
        float offsetCenter = 605.0f;
        float offsetMinus = 550.0f;
        float offsetPlus = 660.0f;

        builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { startX, screenY })
               .FontSize(30.0f).Tint(Colors::BLACK).Text("Plein écran").BuildText();

        UiSprite* cbBg = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetCenter, screenY }).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_CheckBox").BuildSprite();
        UiSprite* cbCheck = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetCenter, screenY }).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_Check").BuildSprite();
        cbCheck->IsEnable = SettingsGame::Get().GetVideo().Fullscreen;

        UiButton* fsB = builder.BuildButton(cbBg);
        if (fsB) {
            fsB->OnClick.ConnectLambda([this, cbCheck]() {
            auto& s = SettingsGame::Get();
            bool next = !s.GetVideo().Fullscreen;
            s.SetFullscreen(next);
            cbCheck->IsEnable = next;
            if (next)
                mp_Scene->GetRenderWindow()->Fullscreen(false);
            else
                mp_Scene->GetRenderWindow()->Resize({1080, 720}, true);
            this->OnSettingsChanged.Emit();
        });
        }

        float fpsY = screenY + 50.0f;
        builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { startX, fpsY })
               .FontSize(30.0f).Tint(Colors::BLACK).Text("Limite de FPS").BuildText();

        UiSprite* fM = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetMinus, fpsY }).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_Minus").BuildSprite();
        UiButton* fMB = builder.BuildButton(fM);

        UiText* fVal = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetCenter, fpsY  +5.0f }).FontSize(30.0f).Tint(Colors::BLACK).Text("60").BuildText();
        fVal->BindReactive(m_FpsValue);

        UiSprite* fP = builder.ChildOf(settingsId).At(Anchors::TOP_LEFT, { offsetPlus, fpsY }).Scale({0.5f, 0.5f}).Sprite("44x43", "Settings_Plus").BuildSprite();
        UiButton* fPB = builder.BuildButton(fP);

        if (fPB) {
            fPB->OnClick.ConnectLambda([this]() {
                int current = SettingsGame::Get().GetVideo().FPS;
                int next = (current >= 240) ? 240 : current + 30;
                SettingsGame::Get().SetFPS(next);
                m_FpsValue.Set(next);

                this->OnSettingsChanged.Emit();
            });
        }

        if (fMB) {
            fMB->OnClick.ConnectLambda([this]() {
                int current = SettingsGame::Get().GetVideo().FPS;
                int next = (current <= 30) ? 30 : current - 30;
                SettingsGame::Get().SetFPS(next);
                m_FpsValue.Set(next);

                this->OnSettingsChanged.Emit();
            });
        }
    }

    void UISettings::Toggle(bool next)
    {
        if (mp_SettingsCanvas) {
            mp_SettingsCanvas->IsEnable = next;
        }
    };
}
