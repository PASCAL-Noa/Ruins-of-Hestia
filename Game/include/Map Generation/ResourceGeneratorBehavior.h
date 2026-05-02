#pragma once
#include "Behavior.h"
#include "MapGenerator.h"
#include "ParticleSystem.h"
#include "Render3DSystem.h"
#include "TweenSystem.h"
#include "UI/UiCanvas.h"
#include "Village/Inventory.h"

namespace GPC {
    struct ResourceBehavior;

    constexpr glm::vec2 TREE_SCALE = { 2.25f, 3.15f };
    constexpr glm::vec2 CAILLOUX_SCALE = { 0.749925f, 1.049895f  };
    constexpr float TREE_OFFSET_Y = 0.75f;
    constexpr float CAILLOUX_OFFSET_Y = 1.25f;

    struct ResourceGeneratorBehavior : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(ResourceGeneratorBehavior);

        GENERATION_ID GenerationID{0};
        Transform3D* pPlayerTransform = nullptr;
        Inventory* pInventory = nullptr;
        UiCanvas* pCanvas = nullptr;

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        struct ResourceComponents {
            glm::ivec2 Room;
            EntityID EID1;
            EntityID EID2;
            Transform3D* pTransform1 = nullptr;
            Transform3D* pTransform2 = nullptr;
            Render3DComponent* pRenderer1 = nullptr;
            Render3DComponent* pRenderer2 = nullptr;
            ResourceBehavior* pBehavior = nullptr;
            TweenComponent* pTweening = nullptr;
            bool UseSprite1 = true;
            bool UseSprite2 = true;
        };

        ResourceComponents* GetClosestResource(float& out_distance, glm::vec2& out_direction);

        void DestroyResource(ResourceComponents* pComponents, Scene* pScene);
        void DestroyAllResources(Scene* pScene);

    private:

        void UpdateVisibilityBasedOnDistance();
        void CreateResource(glm::vec2 position, glm::ivec2 room, Scene *pScene);

        Transform3D* mp_Transform = nullptr;
        glm::ivec2 m_CurrentRoom = { 100, 100 };

        static constexpr uint32_t MAX_RESOURCE_COUNT = 50;
        struct Resources {
            std::array<ResourceComponents, MAX_RESOURCE_COUNT> Array;
            uint32_t Count = 0.0f;
        };
        MAP_ARRAY2D<Resources> m_SpawnedByRoom;
    };

} // GPC