#include "TransiFeuillle.h"

#include "CameraSystem.h"
#include "Scene.h"
#include "TweenSystem.h"

namespace GPC {
    void TransiFeuillle::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);


        auto left = pCtx->pScene->CreateEntityAs3D();

        mp_LeftTransform = pCtx->pScene->GetComponent<Transform3D>(left);
        mp_LeftTransform->LocalTransform.SetScale(1.92f * GENERAL_SCALE_EXPEDITION * PLANE_SCALE.x, 1.0f, 1.08f * GENERAL_SCALE_EXPEDITION * PLANE_SCALE.y);
        mp_LeftTransform->LocalTransform.SetPosition(0.0f, 50.0f, 0.0f);
        mp_LeftTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI);

        mp_LeftRenderer = pCtx->pScene->AddComponent<Render3DComponent>(left);
        mp_LeftRenderer->pGeometry = GPC_MESH("Plane");
        mp_LeftRenderer->SetTexture("1920x1080", "TransiFeuilleLeft");
        mp_LeftRenderer->pGraphicProgram = &GPC_GRAPHIC_PROGRAM("Unlit Program")->Program;
        mp_LeftRenderer->RenderMaterial.Tint = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.7f };

        auto right = pCtx->pScene->CreateEntityAs3D();

        mp_RightTransform = pCtx->pScene->GetComponent<Transform3D>(right);
        mp_RightTransform->LocalTransform.SetScale(1.92f * GENERAL_SCALE_EXPEDITION * PLANE_SCALE.x, 1.0f, 1.08f * GENERAL_SCALE_EXPEDITION * PLANE_SCALE.y);
        mp_RightTransform->LocalTransform.SetPosition(50.0f, 50.0f, 0.0f);
        mp_RightTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI);

        mp_RightRenderer = pCtx->pScene->AddComponent<Render3DComponent>(right);
        mp_RightRenderer->pGeometry = GPC_MESH("Plane");
        mp_RightRenderer->SetTexture("1920x1080", "TransiFeuilleRight");
        mp_RightRenderer->pGraphicProgram = &GPC_GRAPHIC_PROGRAM("Unlit Program")->Program;
        mp_RightRenderer->RenderMaterial.Tint = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.7f };

        mp_CameraTransform = pCtx->pScene->GetComponent<Transform3D>(pCtx->pScene->GetActiveCamera()->GetEntityID());

        mp_TweenComponent = pCtx->pScene->AddComponent<TweenComponent>(GetEntityID());

        mp_LeftRenderer->IsEnable = false;
        mp_RightRenderer->IsEnable = false;
    }

    void TransiFeuillle::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (!m_IsTransitioning && !m_IsOpen
            && mp_LeftRenderer != nullptr && mp_LeftRenderer->IsEnable
            && mp_CameraTransform != nullptr
            && mp_LeftTransform != nullptr && mp_RightTransform != nullptr)
        {
            const glm::vec3 camPos     = mp_CameraTransform->GetWorldPosition();
            const glm::vec3 camForward = mp_CameraTransform->LocalTransform.GetForward();
            mp_LeftTransform->LocalTransform.SetPosition(camPos - camForward * DISTANCE_TO_CAMERA);
            mp_RightTransform->LocalTransform.SetPosition(camPos - camForward * (DISTANCE_TO_CAMERA - 1.0f));
        }

        if (m_IsTransitioning && pCtx->pClock->GetTime() >= m_EndOfTransition) {
            m_IsTransitioning = false;
            if (mp_CallBackOnEndOfTransition != nullptr) {
                mp_CallBackOnEndOfTransition->Call(const_cast<Scene *>(pCtx->pScene));
                delete mp_CallBackOnEndOfTransition;
                mp_CallBackOnEndOfTransition = nullptr;
            }
            if (m_IsOpen) {
                mp_LeftRenderer->IsEnable = false;
                mp_RightRenderer->IsEnable = false;
            }
        }
    }

    bool TransiFeuillle::LaunchOpenAnimation(float time, Scene *pScene, GPC::Callable<void, Scene*>* pCallBack) {
        if (mp_TweenComponent == nullptr || IsTransitioning()) { return false; }

        TweenConfig<float> config{};
        config.Duration = ANIMATION_DURATION;
        config.Start = 0.0f;
        config.End = 1.0f;
        config.Ease = Tweening::EasingType::TransiFeuille;
        config.Setter = [this] (float value) {
            mp_LeftTransform->LocalTransform.SetPosition(
                mp_CameraTransform->GetWorldPosition() - mp_CameraTransform->LocalTransform.GetForward() * DISTANCE_TO_CAMERA - value * mp_CameraTransform->LocalTransform.GetRight() * OFFSET_SCALE
            );

            mp_RightTransform->LocalTransform.SetPosition(
                mp_CameraTransform->GetWorldPosition() - mp_CameraTransform->LocalTransform.GetForward() * (DISTANCE_TO_CAMERA - 1.0f) + value *  mp_CameraTransform->LocalTransform.GetRight() * OFFSET_SCALE
            );
        };

        mp_TweenComponent->AddTween(config);
        m_IsTransitioning = true;
        m_EndOfTransition = time + ANIMATION_DURATION;
        mp_CallBackOnEndOfTransition = pCallBack;
        m_IsOpen = true;
        mp_LeftRenderer->IsEnable = true;
        mp_RightRenderer->IsEnable = true;
        return true;
    }

    void TransiFeuillle::SnapToClosed() {
        if (mp_LeftTransform == nullptr || mp_RightTransform == nullptr) return;
        if (mp_CameraTransform == nullptr) return;

        const glm::vec3 camPos     = mp_CameraTransform->GetWorldPosition();
        const glm::vec3 camForward = mp_CameraTransform->LocalTransform.GetForward();

        mp_LeftTransform->LocalTransform.SetPosition(camPos - camForward * DISTANCE_TO_CAMERA);
        mp_RightTransform->LocalTransform.SetPosition(camPos - camForward * (DISTANCE_TO_CAMERA - 1.0f));

        if (mp_LeftRenderer)  mp_LeftRenderer->IsEnable  = true;
        if (mp_RightRenderer) mp_RightRenderer->IsEnable = true;

        m_IsOpen          = false;
        m_IsTransitioning = false;
    }

    bool TransiFeuillle::LaunchCloseAnimation(float time, Scene *pScene, GPC::Callable<void, Scene*>* pCallBack) {
        if (mp_TweenComponent == nullptr || IsTransitioning()) { return false; }

        TweenConfig<float> config{};
        config.Duration = ANIMATION_DURATION;
        config.Start = 1.0f;
        config.End = 0.0f;
        config.Ease = Tweening::EasingType::EaseInQuad;
        config.Setter = [this] (float value) {
            mp_LeftTransform->LocalTransform.SetPosition(
                mp_CameraTransform->GetWorldPosition() - mp_CameraTransform->LocalTransform.GetForward() * DISTANCE_TO_CAMERA - value * mp_CameraTransform->LocalTransform.GetRight() * OFFSET_SCALE
            );

            mp_RightTransform->LocalTransform.SetPosition(
                mp_CameraTransform->GetWorldPosition() - mp_CameraTransform->LocalTransform.GetForward() * (DISTANCE_TO_CAMERA - 1.0f) + value *  mp_CameraTransform->LocalTransform.GetRight() * OFFSET_SCALE
            );
        };

        mp_TweenComponent->AddTween(config);
        m_IsTransitioning = true;
        m_EndOfTransition = time + ANIMATION_DURATION;
        mp_CallBackOnEndOfTransition = pCallBack;
        m_IsOpen = false;
        mp_LeftRenderer->IsEnable = true;
        mp_RightRenderer->IsEnable = true;
        return true;
    }
} // GPC