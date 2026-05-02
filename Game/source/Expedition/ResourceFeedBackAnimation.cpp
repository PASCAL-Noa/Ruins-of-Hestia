#include "Expedition/ResourceFeedBackAnimation.h"

#include "Scene.h"
#include "TweenSystem.h"
#include "UI/UiAnim.h"
#include "UI/UiBuilder.h"

namespace GPC {

    void ResourceFeedBackAnimation::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);

        glm::vec2 screen_pos = pCtx->pScene->WorldToScreen(ResourcePosition);
        screen_pos = screen_pos * 0.5f;
        GPC_INFO << "Placed at " << screen_pos << ENDL;

        UiBuilder builder{ pCtx->pScene};

        mp_Sprite = builder
            .ChildOf(pCanvas->GetEntityID())
            .At(Anchors::MIDDLE_MIDDLE, {0, 0})
            .Scale(RESOURCE_START_SCALE)
            .Sprite("100x100", ResourceUITextureName)
            .BuildSprite();

        m_StartPosition = mp_Sprite->GetPosition();
        m_EndPosition = pCtx->pScene->GetRenderWindow()->GetSize() * 0.48f;

        UiAnim::MoveTo(*pCtx->pScene, mp_Sprite, m_EndPosition, RESOURCE_ANIMATION_DURATION, Tweening::EasingType::EaseOutExpo);
        UiAnim::ScaleTo(*pCtx->pScene, mp_Sprite, { RESOURCE_END_SCALE, RESOURCE_END_SCALE }, RESOURCE_ANIMATION_DURATION, Tweening::EasingType::EaseOutBounce);
        UiAnim::FadeOut(*pCtx->pScene, mp_Sprite, RESOURCE_ANIMATION_DURATION, Tweening::EasingType::EaseOutBounce);

        m_AnimationHasStarted = true;
    }

    void ResourceFeedBackAnimation::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (m_AnimationHasStarted && UiAnim::IsAnimating(*pCtx->pScene, mp_Sprite->GetEntityID()) == false) {
            // Delete UI correctly ?
            // pCtx->pScene->DestroyEntity(mp_Sprite->GetEntityID());
            mp_Sprite->IsEnable = false;
            pCtx->pScene->DestroyEntity(GetEntityID());
        }

    }
} // GPC