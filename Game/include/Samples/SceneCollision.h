#pragma once

#include "Scenes/SceneDefault.h"

namespace GPC
{
    class SceneCollision : public SceneDefault
    {
        GraphicsProgram* mp_WireFrameProgram = nullptr;
        EntityID m_Entity2;
        EntityID m_RayContactVisualEntity;

        uint32_t m_BlueTextureID;
        uint32_t m_RedTextureID;

        ErrorType OnAssetsLoad() override;
        Seconds startdt;

        ErrorType OnCreate(SceneInformation &info) override;
        void OnDestroy() override;


        EntityID CreateCube(glm::vec3 position, glm::vec3 scale);
        EntityID CreateBoxPartitioning(glm::vec3 position, glm::vec3 scale);
        
        void InitLights();
        void InitProgram();

        void ShowPartiotioningBoxes();

        void LoadTextures();
        void OnSceneUpdate3D() override;
        void OnSceneEndCalculation() override;
    };
}
