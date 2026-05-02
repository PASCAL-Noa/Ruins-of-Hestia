#pragma once

#include "Scene.h"
#include "CameraSystem.h"
#include "TransformComponents.h"
#include "Assets.h"
#include "FollowTarget.h"
#include "FollowTargetVillage.h"
#include "LightingSystem.h"
#include "UI/UiReactive.h"

namespace GPC
{
    struct UiCanvas;
    struct UiText;

    constexpr uint16_t TRONC_COUNT = 7;
    constexpr uint16_t LEAVES_COLOR_COUNT = 6;
    constexpr uint16_t CAILLOUX_COUNT = 12;
    constexpr uint16_t GEM_COUNT = 3;
    constexpr uint16_t PROP_COUNT = 1;
    constexpr uint16_t PROP_LOAD_COUNT = 15;

    class SceneDefault : public Scene
    {

        UiCanvas*  mp_DebugRoot        = nullptr;
        UiText*    mp_FpsLabel         = nullptr;
        UiText*    mp_MouseLabel       = nullptr;
        UiText*    mp_EntityLabel      = nullptr;
        UiText*    mp_LookingLabel     = nullptr;
        UiText*    mp_CameraLabel      = nullptr;

        UiReactive<std::string> m_FpsReactive{ "FPS" };
        UiReactive<std::string> m_MouseReactive{ "MouseCoords" };
        UiReactive<std::string> m_EntityReactive{ "EntityCount" };
        UiReactive<std::string> m_LookingReactive{ "Looking" };
        UiReactive<std::string> m_CameraReactive{ "Camera" };

    protected:

        CameraComponent*    mp_MainCamera = nullptr;
        Transform3D*        mp_MainCameraTransform = nullptr;
        FollowTargetVillage*mp_MainCameraBehaviour = nullptr;

        LightingComponent* mp_DefaultLight = nullptr;

        float           m_Accumulator = 0.0f;
        float           m_FPSSampleCount = 0.0f;

        void LoadTextureAndArrayDefault(const std::string& name, const std::string& filepath);

        void LoadDefaultGraphicShader(
            const std::string& name,
            const std::string& vertexPath,
            const std::string& fragmentPath,
            const std::vector<std::string>& arrays
            );

        void LoadTextureArray(
            const std::string& name,
            float width,
            float height,
            const std::vector<std::pair<std::string, std::string>>& textureNamePaths
        );

    public:

        ErrorType OnAssetsLoad() override;
        void OnAssetsDestroy() override;

        void OnSceneUpdate() override;

        ErrorType OnCreate(SceneInformation &info) override;

        void CreateDebugOverlay();

    };
} // GPC