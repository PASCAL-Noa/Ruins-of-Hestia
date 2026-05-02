#pragma once


#include "SceneDefault.h"

namespace GPC {
    struct DialogueBehaviour;
    struct FreeCamera;
    struct UiCanvas;

    class SceneDialogue : public SceneDefault
    {
    public:
        ErrorType OnAssetsLoad() override;
        ErrorType OnCreate(SceneInformation& info) override;
        void OnSceneUpdate() override;
        void OnDestroy() override;

    private:
        void CreateFreeCamera();
        void CreateSunLighting();
        void CreateGrounds();

        void SetupUI();
    private:
        FreeCamera*            mp_FreeCameraBehavior = nullptr;
        DialogueBehaviour*   mp_DialogueBehaviour  = nullptr;
    };

} // GPC
