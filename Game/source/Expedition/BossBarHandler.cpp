#include "Expedition/BossBarHandler.h"

#include "Expedition/EnnemiBehavior.h"
#include "Expedition/LivingEntityBehavior.h"
#include "UI/UiBuilder.h"

namespace GPC {
    void BossBarHandler::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);

        UiBuilder builder{pCtx->pScene};

        UiCanvas* pCanvas = builder
                .At(Anchors::TOP_LEFT, {0, 0})
                .Tint(Colors::TRANSPARENT)
                .FitToScreen(true)
                .BuildCanvas();

        mp_HealhBackGround = builder
                .ChildOf(pCanvas->GetEntityID())
                .At(Anchors::TOP_LEFT, {241, 50})
                .Scale(SCALE)
                .Sprite("1193x139", "Boss_Health_BG")
                .Tint(Colors::TRANSPARENT)
                .BuildSprite();

        mp_Healh = builder
                .ChildOf(mp_HealhBackGround->GetEntityID())
                .At(Anchors::MIDDLE_MIDDLE, {0, 0 })
                .Scale(SCALE)
                .Sprite("1193x139", "Boss_Health")
                .Tint(Colors::TRANSPARENT)
                .BuildSprite();

        mp_Healh->zIndex = 1001;
        mp_HealhBackGround->zIndex = 1000;

    }

    void BossBarHandler::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (mp_Healh == nullptr) return;

        glm::ivec2 room = WorldPositionToRoomPosition(pBoss->pBehavior->pPlayer->GetWorldPosition());
        glm::ivec2 distance = EndRoom - room;
        if (abs(distance.x) <= 1 && abs(distance.y) <= 1) {
            mp_Healh->SetTint(Colors::WHITE);
            mp_HealhBackGround->SetTint(Colors::WHITE);
            float normlized_health = pBoss->pLiving->CurrentHP / pBoss->pLiving->pMaxHealth->GetFinalValue();
            mp_Healh->SetScale({ normlized_health * SCALE, SCALE });
        }
        else {
            mp_Healh->SetTint(Colors::TRANSPARENT);
            mp_HealhBackGround->SetTint(Colors::TRANSPARENT);
        }
    }

    void BossBarHandler::DisableUI()
    {
        if (mp_Healh) mp_Healh->IsEnable = false;
        if (mp_HealhBackGround) mp_HealhBackGround->IsEnable = false;
    }
} // GPC