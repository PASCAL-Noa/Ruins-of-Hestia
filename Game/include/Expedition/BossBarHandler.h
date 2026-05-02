#pragma once
#include "Behavior.h"
#include "Map Generation/EnnemiGeneratorBehavior.h"
#include "UI/UiSprite.h"

namespace GPC {
    struct UiCanvas;

    struct BossBarHandler : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(BossBarHandler);

        glm::ivec2 EndRoom{0, 0};
        UiCanvas* pMainCanvas = nullptr;
        EnnemiGeneratorBehavior::EnnemiComponents* pBoss = nullptr;

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void DisableUI();

        UiSprite* mp_HealhBackGround = nullptr;
        UiSprite* mp_Healh = nullptr;
    private:

        static constexpr float SCALE = 0.5f;
        static constexpr float DISTANCE_TO_SHOW_HEALTH_BAR = 10.0f * GENERAL_SCALE_EXPEDITION;
        static constexpr float DISTANCE_TO_HIDE_HEALTH_BAR = 20.0f * GENERAL_SCALE_EXPEDITION;

        bool m_Show = false;
    };

} // GPC

