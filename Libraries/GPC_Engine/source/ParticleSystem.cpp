#include "ParticleSystem.h"

#include "Scene.h"

namespace GPC
{
    void ParticleEmitter::AutoStart()
    {
        Play();
    }

    void ParticleEmitter::Play()
    {
        if (Paused) {
            Playing = true;
            return;
        }
        Ended = false;
        Time = PlayAfter + Duration;
        Playing = true;
        pComputeProgram->Reset();
    }

    void ParticleEmitter::Pause()
    {
        Paused = true;
        Playing = false;
    }

    void ParticleEmitter::Stop()
    {
        Playing = false;
        Ended = true;
        Time = 0;
    }

    void ParticleEmitter::SetLooping(bool looping)
    {
        Looping = looping;
    }

    void ParticleEmitter::SetDuration(Seconds duration, Seconds startAfter)
    {
        Duration = duration;
        PlayAfter = startAfter;

        if (PlayAfter > 0) {
            Playing = false;
        }

        Time = PlayAfter + Duration;
    }

    void ParticleSystem::OnBeginUpdate()
    {
        System::OnBeginUpdate();

        CBlockPtr<ParticleEmitter>  particle_component_block = m_Ctx.pScene->GetComponentBlock<ParticleEmitter>();
        ParticleEmitter* particle_components    = particle_component_block->ComponentData();
        uint32_t    entityCount                 = particle_component_block->GetEntityCount();

        Seconds deltaTime = m_Ctx.pClock->GetDeltaTime();
        m_ComputeContext.DeltaTime = deltaTime;

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            if (particle_components[i].IsEnable == false) continue;

            ParticleEmitter& emitter = particle_components[i];
            ComputeProgram* program = emitter.pComputeProgram;

            program->UpdateContext(m_ComputeContext);
            if (!program->IsAvailable()) continue;
            if (emitter.Playing == true) {
                emitter.Time -= deltaTime;
                if (emitter.Time <= 0) {
                    emitter.Playing = false;
                    emitter.Ended = true;
                }
            }

            bool IS_OVER_MODE = emitter.pComputeProgram->GetParticleMode() == ParticleEmissionMode::OVER_TIME;

            if (emitter.Looping == true
                && !emitter.Playing) {

                if (IS_OVER_MODE) {
                    emitter.Playing = true;
                    emitter.Ended = false;
                } else {
                    emitter.Play();
                }
            }

        }

    }

    void ParticleSystem::OnUpdate()
    {
        System::OnUpdate();

        CBlockPtr<ParticleEmitter>  particle_component_block = m_Ctx.pScene->GetComponentBlock<ParticleEmitter>();
        ParticleEmitter* particle_components    = particle_component_block->ComponentData();
        uint32_t    entityCount                 = particle_component_block->GetEntityCount();

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            if (particle_components[i].IsEnable == false) continue;

            ParticleEmitter& emitter = particle_components[i];
            ComputeProgram* program = emitter.pComputeProgram;

            if (!program->IsAvailable()) continue;
            if (!emitter.Playing) continue;

            m_Ctx.pWindow->Calculate(program);

        }
    }

    void ParticleSystem::OnLateUpdate()
    {
        System::OnLateUpdate();

        CBlockPtr<ParticleEmitter>  particle_component_block = m_Ctx.pScene->GetComponentBlock<ParticleEmitter>();
        ParticleEmitter*            particle_components = particle_component_block->ComponentData();
        EntityID*   entities = particle_component_block->EntitiesData();
        uint32_t    entityCount = particle_component_block->GetEntityCount();

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            if (particle_components[i].IsEnable == false) continue;
            if (particle_components[i].Ended == true) continue;

            if (!particle_components[i].pComputeProgram->IsAvailable()) continue;
            Transform3D* transform = m_Ctx.pScene->GetComponent<Transform3D>(entities[i]);

            m_ParticleObject.Material_ = &particle_components[i].RenderMaterial;
            m_ParticleObject.ComputeProgram_ = particle_components[i].pComputeProgram;
            m_ParticleObject.Mesh = particle_components[i].pGeometry;
            m_ParticleObject.Transform = &transform->GetWorldMatrix();

            m_Ctx.pWindow->DrawParticles(m_ParticleObject);

        }

    }
} // GPC