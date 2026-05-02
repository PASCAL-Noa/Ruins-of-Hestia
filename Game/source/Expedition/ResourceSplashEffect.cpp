#include "Expedition/ResourceSplashEffect.h"

#include "Scene.h"

namespace GPC {
    void ResourceSplashEffect::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);

        pCtx->pScene->GetComponent<Transform3D>(GetEntityID())->LocalTransform.SetPosition(WorldPosition);

        mp_Emitter = pCtx->pScene->AddComponent<ParticleEmitter>(GetEntityID());
        mp_Emitter->pGeometry = GPC_MESH("Plane");
        mp_Emitter->pComputeProgram = &GPC_COMPUTE_PROGRAM(ComputeName)->Program;
        mp_Emitter->SetLooping(false);
        mp_Emitter->SetDuration(SPLASH_DURATION, 0.0f);
        mp_Emitter->Play();

        m_TimeShouldSucide = pCtx->pClock->GetTime() + SPLASH_DURATION;

    }

    void ResourceSplashEffect::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (pCtx->pClock->GetTime() >= m_TimeShouldSucide) {
            pCtx->pScene->DestroyEntity(GetEntityID());
        }

    }
} // GPC