#pragma once
#include "SceneDefault.h"

namespace GPC {

    struct UiCanvas;

    class SceneMadeWith : public SceneDefault
    {
    public:
        ErrorType OnAssetsLoad() override;
        ErrorType OnCreate(SceneInformation& info) override;
        ErrorType OnStart(SceneChangeCtx &ctx) override;
        void OnSceneUpdate() override;
        void OnDestroy() override;

    private:
        UiCanvas* mp_rootCanva = nullptr;
        UiSprite* mp_Text1 = nullptr;
        UiSprite* mp_Text2 = nullptr;
        UiSprite* mp_Sprite = nullptr;

        SceneChangeCtx m_ctx;
        int m_AnimFinishedCount = 0;
        bool m_IsAnimEnded = false;
    };

} // GPC
