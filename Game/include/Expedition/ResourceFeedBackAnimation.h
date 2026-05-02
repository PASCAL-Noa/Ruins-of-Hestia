#pragma once
#include "Behavior.h"
#include "UI/UiSprite.h"

namespace GPC {
    struct UiCanvas;

    struct ResourceFeedBackAnimation : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(ResourceFeedBackAnimation);

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        glm::vec3 ResourcePosition;
        UiCanvas* pCanvas = nullptr;
        std::string ResourceUITextureName = "Pickup_Wood";
    private:
        glm::vec2 m_StartPosition = { 0.0f, 0.0f };
        glm::vec2 m_EndPosition = { 0.0f, 0.0f };
        UiSprite* mp_Sprite = nullptr;
        bool m_AnimationHasStarted = false;

        static constexpr float RESOURCE_ANIMATION_DURATION = 1.0f;
        static constexpr float RESOURCE_START_SCALE = 1.0f;
        static constexpr float RESOURCE_END_SCALE = 0.1f;
    };

} // GPC