#include "Expedition/HealthBarController.h"

#include "Map Generation/EnnemiGeneratorBehavior.h"

namespace GPC {
    void HealthBarController::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (pComponents == nullptr) return;

        if (pComponents->pLiving != nullptr)
        {
            float normalized_hp = pComponents->pLiving->CurrentHP / pComponents->pLiving->pMaxHealth->GetFinalValue();
            pComponents->pHealthBarTransform->LocalTransform.SetScale(normalized_hp * BaseScale, 1.0f, SCALE_Y_HEALTH);
            pComponents->pHealthBarTransform->LocalTransform.SetPosition(
                pComponents->pTransform->GetWorldPosition() + glm::vec3(0, HEIGHT, 0)
            );
        }

    }
} // GPC