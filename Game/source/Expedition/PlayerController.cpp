#include "../../include/Expedition/PlayerController.h"

#include "Inputs.h"
#include "Scene.h"

namespace GPC {
    void PlayerController::UseRunningAnimation() {
        if (CurrentAnimation == PlayerAnimation::RUNNING) return;
        mp_Animation->ChangeAnimationTo("Running");
        mp_Animation->Speed = 1.83f;
        CurrentAnimation = PlayerAnimation::RUNNING;
    }

    void PlayerController::UseIdleAnimation() {
        if (CurrentAnimation == PlayerAnimation::IDLE) return;
        mp_Animation->ChangeAnimationTo("Idle");
        mp_Animation->Speed = 1.0f;
        CurrentAnimation = PlayerAnimation::IDLE;
    }

    void PlayerController::UsePunchingAnimation() {
        if (CurrentAnimation == PlayerAnimation::PUNCHING) return;
        mp_Animation->ChangeAnimationTo("Punching");
        mp_Animation->Speed = 1.0f;
        CurrentAnimation = PlayerAnimation::PUNCHING;
    }

    void PlayerController::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);

        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
        mp_SlidingAudio = pCtx->pScene->GetComponent<AudioSource>(GetEntityID());
        mp_TweenComponent = pCtx->pScene->GetComponent<TweenComponent>(GetEntityID());
        mp_Collider = pCtx->pScene->GetComponent<Collision3DComponent>(GetEntityID());
        mp_Animation = pCtx->pScene->GetComponent<AnimationComponent>(GetEntityID());
        mp_Renderer = pCtx->pScene->GetComponent<Render3DComponent>(GetEntityID());
    }

    void PlayerController::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (pCtx->pClock->GetTimeScale() < FLT_EPSILON) return;

        constexpr auto cos45 = 0.70710678118f;
        constexpr auto sin45 = 0.70710678118f;

        glm::vec2 direction{0, 0};
        IsMoving = false;
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::W)) {
            direction.x += cos45;
            direction.y += sin45;
            IsMoving = true;
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::S)) {
            direction.x -= cos45;
            direction.y -= sin45;
            IsMoving = true;
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::A)) {
            direction.x += cos45;
            direction.y -= sin45;
            IsMoving = true;
        }
        if (GPC::Inputs::IsKeyDown(GPC::Inputs::KeyCode::D)) {
            direction.x -= cos45;
            direction.y += sin45;
            IsMoving = true;
        }
        if (direction.x == 0.0f && direction.y == 0.0f) IsMoving = false;

        if (IsMoving) {
            direction = glm::normalize(direction);
            if (glm::dot(CurrentDirection, direction) < -0.5f) CurrentDirection = -CurrentDirection;
            float t = 1.0f - glm::exp(-TURN_SPEED * pCtx->pClock->GetDeltaTime());
            CurrentDirection = CurrentDirection * (1 - t) + direction * t;
            CurrentDirection = glm::normalize(CurrentDirection);
            CurrentOrientation = glm::atan2(CurrentDirection.y, CurrentDirection.x);
            UseRunningAnimation();
        } else {
            float t = 1.0f - glm::exp(-TURN_SPEED * pCtx->pClock->GetDeltaTime());
            CurrentDirection = CurrentDirection * (1 - t) + glm::vec2{0, 0} * t;
            UseIdleAnimation();
        }

        if (!IsDashing && GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::SPACE)) {

            if (pSlidePlayer) pSlidePlayer->Play();

            auto last_value = std::make_shared<float>(0.0f);
            glm::vec2 dash_direction = {
                cos(CurrentOrientation),
                sin(CurrentOrientation)
            };
            TweenConfig<float> config;
            config.Duration = DASH_DURATION;
            config.Start = 0.0f;
            config.End = DASH_DISTANCE;
            config.Ease = Tweening::EasingType::EaseOutQuad;
            config.Setter = [this, dash_direction, last_value](float value) {
                float diff = value - *last_value;
                mp_Transform->LocalTransform.AddPosition(glm::vec3{dash_direction.x, 0.0f, dash_direction.y} * diff);
                *last_value = value;
            };
            mp_TweenComponent->AddTween(config);
            IsDashing = true;
            IsInvulnerable = true;
            DashingStartTime = pCtx->pClock->GetTime();
            return;
        }

        float TimeSinceDashingStart = pCtx->pClock->GetTime() - DashingStartTime;
        if (TimeSinceDashingStart > INVULNERABILITY_DURATION)
            IsInvulnerable = false;
        if (IsDashing) {
            if ( TimeSinceDashingStart >= DASH_DURATION) IsDashing = false;
        }

        if (IsMoving) {
            CurrentSpeed = glm::lerp<float>(CurrentSpeed, MaxSpeed, pCtx->pClock->GetDeltaTime() * Acceleration);
            mp_Transform->LocalTransform.SetRotationYawPitchRoll(
                -CurrentOrientation + GPC_PI * 0.5f,
                0,
                0
            );
            m_FootstepTimer -= pCtx->pClock->GetDeltaTime();
            if (m_FootstepTimer <= 0.0f)
            {
                if (pFootstepPlayer) pFootstepPlayer->Play();
                m_FootstepTimer = 0.35f;
            }
        }
        else {
            CurrentSpeed = glm::lerp<float>(CurrentSpeed, 0, pCtx->pClock->GetDeltaTime() * Deceleration);
            m_FootstepTimer = 0.0f;
        }

        if (IsDashing && pSlidePlayer != nullptr) {
        }

        mp_Transform->LocalTransform.AddPosition({
            CurrentDirection.x * pCtx->pClock->GetDeltaTime() * CurrentSpeed,
            0,
            CurrentDirection.y * pCtx->pClock->GetDeltaTime() * CurrentSpeed
        });

    }
} // GPC