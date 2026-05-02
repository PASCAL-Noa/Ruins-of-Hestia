#pragma once

#include "Behavior.h"
#include "Equipment.h"
#include "Map Generation/ResourceGeneratorBehavior.h"

namespace GPC {

    struct ResourceBehavior : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(ResourceBehavior);

        ResourceGeneratorBehavior* pManager = nullptr;
        ResourceGeneratorBehavior::ResourceComponents* pComponents = nullptr;

        bool TryCollect(ResourceGeneratorBehavior::ResourceComponents* pResource, Scene* pScene, float time);

        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        static constexpr float COOLDOWN_COLLECT_DURATION = 0.4f;
        static constexpr glm::vec3 SCALE_OFFSET_RESOURCE1 = glm::vec3{ -0.3f, 0.0f, 0.3f } * GENERAL_SCALE_EXPEDITION;
        static constexpr glm::vec3 SCALE_OFFSET_RESOURCE2 = glm::vec3{ -0.075f, 0.0f, 0.075f } * GENERAL_SCALE_EXPEDITION;

        void InitAs(Tier resource_tier, ResourceType resource_type, Scene* pScene);

        void InitVisualAsWood(Scene* pScene);
        void InitVisualAsStone(Scene* pScene);

        void InitVisualAsBlueGem(Scene* pScene);
        void InitVisualAsRedGem(Scene* pScene);
        void InitVisualAsYellowGem(Scene* pScene);
        void InitVisualAsGreenGem(Scene* pScene);
        void InitVisualAsOrangeGem(Scene* pScene);
        void InitVisualAsPurpleGem(Scene* pScene);

        float GetIndicatorOffsetY();

        Tier m_ResourceTier              = 1;
        ResourceType m_ResourceType     = ResourceType::WOOD;
        uint32_t m_ResourceQte          = 1;
    private:
        ResourceID GetResourceID();
        template<typename Resource_t>
        static ResourceID GetResourceIDWithTier(Tier tier);

        void CreateUIFeedBackResource(Scene* pScene);

        glm::vec3 m_BaseScaleResource1{1.0f, 1.0f, 1.0f};
        glm::vec3 m_BaseScaleResources2{1.0f, 1.0f, 1.0f};

        uint32_t m_HitRemaining = 3;


        bool m_IsBeingCollected = false;
        float m_TimeShouldBeCollectable = 0.0f;


    };

} // GPC