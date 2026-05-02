#pragma once

#include "Behavior.h"
#include "CameraSystem.h"
#include "Render3DSystem.h"
#include "RoomMeta.h"
#include "Map Generation/MapGenerator.h"

namespace GPC {
    struct RoomEditorBehavior;

    struct RoomPreviewBehavior : public Behavior {

        INHERIT_BEHAVIOR_CONSTRUCTOR(RoomPreviewBehavior);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void GenerateWith(RoomMeta* pMeta, Scene *pScene);

        RoomEditorBehavior* pEditor = nullptr;
    private:

        void CreateTerrain(Scene* pScene);
        void CreateCamera(Scene* pScene);

        void ClearEditor(Scene* pScene);

        void GenerateTerrain(Scene* pScene);
        void GenerateEnnemiSpawners(Scene* pScene);
        void GenerateResourceSpawners(Scene* pScene);
        void GeneratePlayerSpawner(Scene* pScene);

        Transform3D* mp_Transform = nullptr;

        Render3DComponent* mp_TerrainRenderer;

        Transform3D* mp_CameraTransform = nullptr;
        CameraComponent* mp_CameraComponent = nullptr;

        GENERATION_ID m_Generation;
        const RoomMeta* mp_MetaRoom;
        std::vector<EntityID> m_LivingSpawner;
    };

}
