#pragma once

#include "Behavior.h"
#include "MapGenerator.h"
#include "Render3DSystem.h"
#include "TransformComponents.h"
#include "RoomEditor/RoomMeta.h"
#include "Scenes/SceneDefault.h"

namespace GPC {
    class SceneExpedition;


    class TerrainGeneratorBehavior : public Behavior {
        static constexpr float BORDER_HEIGHT = 1.5f * GENERAL_SCALE_EXPEDITION;

        Transform3D* mp_Transform;

        struct RoomComponents {
            Transform3D* pTransform = nullptr;
            Render3DComponent* pRenderer = nullptr;
        };
        std::vector<Render3DComponent*> mp_CollisionRenderer;
        MAP_ARRAY2D<RoomComponents> m_RoomComponents;
        glm::vec2 m_PlayerSpawn{0.0f, 0.0f};
        bool m_IsGenerated = false;

        void CreateBorder(const GPC::MapGeneratorData::MapBorder& border, Scene* pScene);
        void CreateRoom(RoomData& room_data, Geometry* pGeometry, Scene *pScene);
        void CreateBushes(Scene* pScene);
    public:
        static constexpr uint32_t PROP_COUNT_BY_TYPE = 350;
        std::array<Batching3DComponent*, PROP_COUNT>* pProps;
        GENERATION_ID GenerationID{0};
        Transform3D* pPlayerTransform = nullptr;

        INHERIT_BEHAVIOR_CONSTRUCTOR(TerrainGeneratorBehavior);

        void OnStart(const BehaviorCreateContext *pCtx) override;

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;
        void OnDestroy() override;

        void DestroyTerrain(Scene* pScene) const;
    };

} // GPC