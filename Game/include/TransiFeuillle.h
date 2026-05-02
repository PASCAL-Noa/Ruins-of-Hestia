#pragma once
#include "Behavior.h"
#include "Render3DSystem.h"
#include "TweenSystem.h"

namespace GPC {

    struct TransiFeuillle : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(TransiFeuillle);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        bool LaunchOpenAnimation(float time, Scene* pScene, GPC::Callable<void, Scene*>* pCallBack = nullptr);
        bool LaunchCloseAnimation(float time, Scene* pScene, GPC::Callable<void, Scene*>* pCallBack = nullptr);

        void SnapToClosed();

        bool IsTransitioning() const { return m_IsTransitioning; }

private:
        bool m_IsTransitioning = false;
        bool m_IsOpen = true;
        float m_EndOfTransition = 0.0f;

        Transform3D* mp_CameraTransform     = nullptr;

        Transform3D* mp_LeftTransform       = nullptr;
        Transform3D* mp_RightTransform      = nullptr;
        Render3DComponent* mp_LeftRenderer  = nullptr;
        Render3DComponent* mp_RightRenderer = nullptr;
        TweenComponent* mp_TweenComponent = nullptr;
        GPC::Callable<void, Scene*>* mp_CallBackOnEndOfTransition = nullptr;

        static constexpr float DISTANCE_TO_CAMERA = 75.0f;
        static constexpr float OFFSET_SCALE = 75.0f;
        static constexpr float ANIMATION_DURATION = 1.75f;
        static constexpr glm::vec2 PLANE_SCALE = { 5.0f, 4.0f };

    };

} // GPC

