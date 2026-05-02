#pragma once
#include "Component.h"
#include "Geometry.h"
#include "Material.h"
#include "RenderObjectParticle.h"
#include "System.h"

namespace GPC
{
    class ParticleSystem ;

    struct ParticleEmitter : public Component {
        using Seconds = double;

        BIND_COMPONENT(PARTICLE);
        AUTO_COMPONENT_CONSTRUCTOR(ParticleEmitter);

        Material RenderMaterial{};
        Geometry* pGeometry = nullptr;
        ComputeProgram* pComputeProgram = nullptr;

        void AutoStart();
        void Play();
        void Pause();
        void Stop();

        void SetLooping(bool looping);
        void SetDuration(Seconds duration, Seconds startAfter);

    private:
        bool    Playing = false;
        bool    Ended   = false;
        bool    Paused  = false;
        bool    Looping = false;

        double  Time        = 1.0f;
        double  PlayAfter   = 0.0f;
        double  Duration    = 1.0f;

        friend ParticleSystem;

    };

    class ParticleSystem : public System {

        GPC::RenderObjectParticle   m_ParticleObject{};
        ComputeContext              m_ComputeContext{};

    public:
        BIND_SYSTEM(PARTICLE);

        void OnBeginUpdate() override;
        void OnUpdate() override;
        void OnLateUpdate() override;
    };
} // GPC