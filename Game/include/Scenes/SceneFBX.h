#pragma once

#include "SceneDefault.h"

namespace GPC
{
    struct FbxMeshData;

    class SceneFBX : public SceneDefault
    {
    public:
        ErrorType OnAssetsLoad() override;
        void OnSceneUpdate() override;
        ErrorType OnCreate(SceneInformation &info) override;
        void OnDestroy() override;

    private:
        FbxMeshData* mp_FbxData = nullptr;
        EntityID m_AnimationEntity = 0;
    };
}
