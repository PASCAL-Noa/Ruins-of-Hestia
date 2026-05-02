#pragma once
#include "ECS_Defines.h"
#include <glm/vec2.hpp>
#include <vector>
#include <unordered_map>
#include "Scene.h"
#include "Buildings/BuildingBehavior.h"
#include "BuildingData.h"

namespace GPC
{
    class BuildingManager
    {
    public:
        BuildingManager(Scene* scene, VillageManager* manager);
        ~BuildingManager();

        void        BuildingInitializer(const std::vector<BuildingData>& currentData);
        void        BuildingCleaner();
        EntityID    SpawnBuildingFromType(BuildingType type, glm::ivec2 position, BuildingBehavior** buildingBehavior);

        void        StashEntity(EntityID id, BuildingType type);
        EntityID    UnstashEntity(BuildingType type, glm::ivec2 newPos);

    private:
        Scene* m_Scene;
        VillageManager* m_Manager;
        std::vector<EntityID> m_SpawnedEntities;
        std::unordered_map<BuildingType, std::vector<EntityID>> m_StashedPool;

        template<typename B_Class, typename Behavior>
        EntityID SpawnBuilding(glm::ivec2 position, BuildingBehavior** out) const;

        float GetMapGroundYAt(float worldX, float worldZ) const;
    };
}

#include "BuildingManager.inl"