#include "RoomEditor/SpawnerDebug.h"

#include "CameraSystem.h"
#include "Inputs.h"
#include "Scene.h"

namespace GPC {
    void SpawnerDebug::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (GPC::Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_RIGHT)) {
            auto* camera = pCtx->pScene->GetActiveCamera();
            auto* cTransform = pCtx->pScene->GetComponent<Transform3D>(camera->GetEntityID());
            Ray ray{ cTransform->LocalTransform.GetPosition(), -cTransform->LocalTransform.GetForward() };
            RayHit hit;
            if (pCtx->pScene->RayCast3D(hit, ray)) {
                if (hit.HitEntity == GetEntityID()) {
                    pCtx->pScene->DestroyEntity(GetEntityID());
                }
            }
        }

    }
} // GPC