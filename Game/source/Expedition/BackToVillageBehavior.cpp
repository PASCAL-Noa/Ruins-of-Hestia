#include "Expedition/BackToVillageBehavior.h"

#include "Inputs.h"
#include "Scene.h"
#include "Expedition/TutoBehaviors.h"
#include "Scenes/SceneVillageV1.h"

namespace GPC {
    void BackToVillageBehavior::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);

        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
    }

    void BackToVillageBehavior::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        glm::vec3 diff = pPlayerTransform->GetWorldPosition() - mp_Transform->GetWorldPosition();
        float distance = glm::length(diff);

        if (distance < glm::max(mp_Transform->LocalTransform.GetScale().x, mp_Transform->LocalTransform.GetScale().z)) {
            if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::E)) {
                if (OnInteract) OnInteract();
            }
        }
        else if (distance < 15.0f * GENERAL_SCALE_EXPEDITION) {
            if (UseTuto && m_EndOfMissionTutoHasProc == false
                && pCtx->pClock->GetTime() > Start_Time + 3.1f
                && TutoQueue::CanSpawnTuto(static_cast<float>(pCtx->pClock->GetTime()))) {
                auto tuto1 = pCtx->pScene->CreateEntityAs3D();
                pCtx->pScene->AddBehavior<ExpeditionEndOfMissionTuto>(tuto1);
                m_EndOfMissionTutoHasProc = true;
            }
        }

    }
} // GPC
