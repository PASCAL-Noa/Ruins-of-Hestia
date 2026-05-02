#include "Expedition/IrisFollowing.h"

#include "AnimationComponent.h"
#include "Scene.h"

namespace GPC {
    void IrisFollowing::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);

        CreateIris(pCtx->pScene);

        m_FollowedPosition = pPlayerTransform->GetWorldPosition();

    }

    void IrisFollowing::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (mp_Transform == nullptr) return;

        float time = pCtx->pClock->GetTime() - m_LastTimePositionRegistered;
        if (time >= COOLDOWN_BETWEEN_UPDATE) {
            glm::vec3 cali_position = pPlayerTransform->GetWorldPosition();

            if (m_CaliPositionsRegistered.empty() || glm::distance(cali_position, m_CaliPositionsRegistered.back()) > IRIS_MIN_DISTANCE_TO_CALI) {
                m_CaliPositionsRegistered.push(cali_position);
            }

            m_LastPosition = m_FollowedPosition;
            if (m_CaliPositionsRegistered.size() > MIN_REGISTERED_BEFORE_FOLLOWING) {
                m_FollowedPosition = m_CaliPositionsRegistered.front();
                m_CaliPositionsRegistered.pop();
                UseRunningAnimation();
            } else {
                UseIdleAnimation();
            }

            m_Direction = glm::vec2(m_FollowedPosition.x, m_FollowedPosition.z) - glm::vec2(m_LastPosition.x, m_LastPosition.z);
            if (glm::abs(m_Direction.x) > FLT_EPSILON || glm::abs(m_Direction.y) > FLT_EPSILON) {
                mp_Transform->LocalTransform.SetRotationYawPitchRoll(
                    -glm::atan2(m_Direction.y, m_Direction.x) + GPC_PI * 0.5f,
                    0,
                    0
                );
            }

            m_LastTimePositionRegistered = pCtx->pClock->GetTime();
            time = 0;
        }

        float normalized_time = time / COOLDOWN_BETWEEN_UPDATE;
        mp_Transform->LocalTransform.SetPosition(
            normalized_time * m_FollowedPosition + (1.0 - normalized_time) * m_LastPosition
        );

    }

    void IrisFollowing::UseRunningAnimation() {
        if (m_CurrentAnimation == IrisAnimation::Run) return;
        mp_AnimationComponent->ChangeAnimationTo("Run");
        mp_AnimationComponent->Speed = 1.0f;
        m_CurrentAnimation = IrisAnimation::Run;
    }

    void IrisFollowing::UseIdleAnimation() {
        if (m_CurrentAnimation == IrisAnimation::Idle) return;
        mp_AnimationComponent->ChangeAnimationTo("Idle");
        mp_AnimationComponent->Speed = 1.0f;

        m_CurrentAnimation = IrisAnimation::Idle;
    }

    void IrisFollowing::CreateIris(Scene *pScene) {
        mp_Transform = pScene->GetComponent<Transform3D>(GetEntityID());
        mp_Transform->LocalTransform.SetScale(0.0015 * GENERAL_SCALE_EXPEDITION, 0.0015 * GENERAL_SCALE_EXPEDITION, 0.0015 * GENERAL_SCALE_EXPEDITION);
        mp_Transform->LocalTransform.SetRotationYawPitchRoll(GPC_PI, 0, 0);

        auto renderer = pScene->AddComponent<Render3DComponent>(GetEntityID());
        renderer->pGeometry = GPC_MESH("Iris");
        renderer->SetTexture("2048x2048", "IrisTexture");

        mp_AnimationComponent = pScene->AddComponent<AnimationComponent>(GetEntityID());
        mp_AnimationComponent->SetFbxData(pIrisMesh);
        UseIdleAnimation();
    }
} // GPC