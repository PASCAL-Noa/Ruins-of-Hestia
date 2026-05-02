#include "Expedition/HitBox.h"

#include "Scene.h"
#include "Expedition/PlayerInteraction.h"

namespace GPC {
    void HitBox::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (pCtx->pClock->GetTime() - Descriptor.StartTime > Descriptor.Duration) {
            pCtx->pScene->DestroyEntity(GetEntityID());
        }
    }

    void HitBox::OnCollision3D(const BehaviorCollision3DContext *pCtx) {
        Behavior::OnCollision3D(pCtx);

        const bool ALREADY_HIT = EnnemiHit.contains(pCtx->Collided.EID);
        if (ALREADY_HIT) return;

        LivingEntityBehavior* pLivingDest = pCtx->pScene->GetBehavior<LivingEntityBehavior>(pCtx->Collided.EID);
        if (pLivingDest == nullptr) return;

        bool IsPlayer = pCtx->Collided.pCollider->CollType == ColliderType::Player;
        if (Descriptor.TargetPlayer) {
            int a = 0;
        }

        const bool IS_TARGET = IsPlayer == Descriptor.TargetPlayer;
        const bool SHOULD_BE_HIT = !(Descriptor.DestroyOnHit && HasHit);
        if (IS_TARGET && SHOULD_BE_HIT && pCtx->pScene->IsAlive(Descriptor.pSourceLiving->GetEntityID())) {
            pLivingDest->Attack(Descriptor.pSourceLiving, Descriptor.IsMelee, pCtx->pScene);
            EnnemiHit.insert(pCtx->Collided.EID);
            if (Descriptor.DestroyOnHit) {
                pCtx->pScene->DestroyEntity(GetEntityID());
            }
            HasHit = true;
        }
    }

} // GPC