#pragma once
#include "Scene.h"
#include "../Scenes/SceneDefault.h"

namespace GPC
{
    struct CameraComponent;

    class SceneGraphics : public SceneDefault
    {
    public:

        ErrorType OnAssetsLoad() override;
        void OnAssetsDestroy() override;

        ErrorType OnCreate(SceneInformation &info) override;
        void OnDestroy() override;

    };
} // GPC