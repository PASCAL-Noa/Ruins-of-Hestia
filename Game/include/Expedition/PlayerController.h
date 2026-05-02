#pragma once

#include "AnimationComponent.h"
#include "AudioSystem.h"
#include "Behavior.h"
#include "Render3DSystem.h"
#include "TransformComponents.h"
#include "TweenSystem.h"
#include "RandomAudio.h"

namespace GPC {

    class PlayerController : public Behavior {
        Transform3D* mp_Transform;

        void UseRunningAnimation();
        void UseIdleAnimation();
        void UsePunchingAnimation();

        Collision3DComponent* mp_Collider = nullptr;
        AudioSource* mp_SlidingAudio = nullptr;
        TweenComponent* mp_TweenComponent = nullptr;
        AnimationComponent* mp_Animation = nullptr;
        Render3DComponent* mp_Renderer = nullptr;

    public:
        float MaxSpeed = 3.0f * GENERAL_SCALE_EXPEDITION;
        float Acceleration = 2.0f;
        float Deceleration = 5.0f;

        glm::vec2 CurrentDirection = { 0.0f, 0.0f };
        Radian CurrentOrientation = 0.0f;
        float CurrentSpeed = 0.0f;
        bool IsMoving = false;

        bool IsDashing = false;
        bool IsInvulnerable = false;
        float DashingStartTime = 0.0f;

        static constexpr float TURN_SPEED = 12.0f;
        static constexpr float DASH_DURATION = 0.8f;
        static constexpr float DASH_DISTANCE = 1.8f * GENERAL_SCALE_EXPEDITION;
        static constexpr float INVULNERABILITY_DURATION = 0.35f;

        enum class PlayerAnimation {
            NONE,
            IDLE,
            RUNNING,
            PUNCHING
        };
        PlayerAnimation CurrentAnimation = PlayerAnimation::NONE;
        FbxMeshData* pCaliFbxData;

        RandomAudio* pFootstepPlayer = nullptr;
        RandomAudio* pSlidePlayer    = nullptr;
        float m_FootstepTimer = 0.0f;

        INHERIT_BEHAVIOR_CONSTRUCTOR(PlayerController);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;
    };

} // GPC