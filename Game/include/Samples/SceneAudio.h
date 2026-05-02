#pragma once

#include "ErrorType.h"
#include "Scene.h"
#include "Scenes/SceneDefault.h"

namespace GPC
{
    class SceneAudio : public SceneDefault
    {
    public:
        ErrorType OnAssetsLoad() override;

        ErrorType OnCreate(SceneInformation &info) override;

        void OnSceneUpdate() override;

    private:
        EntityID m_MusicEntity = 0;
        EntityID m_SfxEntity   = 0;
    };
} // GPC