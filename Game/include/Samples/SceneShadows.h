#pragma once
#include "Scene.h"
#include "../Scenes/SceneDefault.h"
#include "FollowTarget.h"
#include "Expedition/PlayerController.h"

namespace GPC
{
    struct CameraComponent;

    class SceneShadows : public SceneDefault
    {
    public:
        ErrorType OnAssetsLoad() override;
        ErrorType OnCreate(SceneInformation &info) override;
        void OnSceneUpdate() override;
        void OnDestroy() override;

        FollowTarget* mp_MainCameraBehavior = nullptr;
        PlayerController* mp_PlayerControllerBehavior = nullptr;
    };
} // GPC