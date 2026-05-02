#pragma once

#include "ParticleSystem.h"
#include "Scene.h"


namespace GPC
{

    class SceneCompute: public Scene
    {
        ComputeProgram* mp_ComputeProgramBurst;
        ComputeProgram* mp_ComputeProgramOverTime;

        Transform3D* m_BaseTransform;
        Transform3D* m_BaseTransform2;
        ParticleEmitter* m_Emmiter;
        ParticleEmitter* m_Emmiter2;

    public:

        ErrorType OnAssetsLoad() override;
        void OnAssetsDestroy() override;

        ErrorType OnCreate(SceneInformation &info) override;
        void OnSceneUpdate() override;
        void OnDestroy() override;

    };

}
