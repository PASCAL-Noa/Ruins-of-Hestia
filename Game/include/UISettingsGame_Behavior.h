#pragma once

#include "UI/UiReactive.h"
#include "UI/UiSignal.h"
#include "UI/UiBuilder.h"

namespace GPC
{
    class UISettings : public Behavior
    {
        public:
        INHERIT_BEHAVIOR_CONSTRUCTOR(UISettings);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnStart(const BehaviorCreateContext* pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void Toggle(bool next);

        UiCanvas*               mp_SettingsCanvas = nullptr;
        UiSprite*               mp_SettingsSprite = nullptr;
        UiButton*               mp_SettingsCloseButton = nullptr;

        UiSignal<>              OnSettingsChanged;
        UiSignal<>              OnCloseRequested;

        UiReactive<int>         m_MasterVolume{ 100 };
        UiReactive<int>         m_MusicVolume{ 100 };
        UiReactive<int>         m_SfxVolume{ 100 };

        UiReactive<int>         m_FpsValue{ 60 };

        private:

        void BuildVolumeUI(UiBuilder& b);
        void BuildGraphicsUI(UiBuilder& b);

        Scene* mp_Scene = nullptr;
    };
}