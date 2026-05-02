#include "Village/BuildingManager.h"
#include "Village/Buildings/LifeTree/LifeTree.h"
#include "Village/Buildings/Farm/Farm.h"
#include "Village/Buildings/Hospital/Hospital.h"
#include "Village/Buildings/Forge/Forge.h"
#include "Village/Buildings/House/House.h"
#include "Village/VillageManager.h"
#include "Village/Map/Map.h"

namespace GPC
{
    BuildingManager::BuildingManager(Scene* scene, VillageManager* manager)
        : m_Scene(scene), m_Manager(manager)
    {
    }

    BuildingManager::~BuildingManager()
    {
        BuildingCleaner();
    }

    void BuildingManager::BuildingInitializer(const std::vector<BuildingData>& currentData)
    {
        BuildingCleaner();
        for (const BuildingData& data : currentData)
        {
            BuildingBehavior* buildingBehavior = nullptr;
            EntityID newEntity = SpawnBuildingFromType(data.Type, data.Position, &buildingBehavior);

            if (data.IsStashed && newEntity != 0)
            {
                StashEntity(newEntity, data.Type);
            }
        }
    }

    void BuildingManager::BuildingCleaner()
    {
        m_SpawnedEntities.clear();
        m_StashedPool.clear();
    }

    EntityID BuildingManager::SpawnBuildingFromType(BuildingType type, glm::ivec2 position, BuildingBehavior** buildingBehavior)
    {
        EntityID newEntity = 0;

        switch (type)
        {
            case BuildingType::LifeTree:    newEntity  = SpawnBuilding<LifeTree, LifeTreeBehavior   >(position, buildingBehavior); break;
            case BuildingType::Farm:        newEntity  = SpawnBuilding<Farm    , FarmBehavior       >(position, buildingBehavior);  break;
            case BuildingType::Hospital:    newEntity  = SpawnBuilding<Hospital, HospitalBehavior   >(position, buildingBehavior); break;
            case BuildingType::Forge:       newEntity  = SpawnBuilding<Forge   , ForgeBehavior      >(position, buildingBehavior);    break;
            case BuildingType::House:       newEntity  = SpawnBuilding<House   , HouseBehaviour     >(position, buildingBehavior); break;
            default: break;
        }

        if (newEntity != 0)
        {
            m_SpawnedEntities.push_back(newEntity);
        }

        return newEntity;
    }

    void BuildingManager::StashEntity(EntityID id, BuildingType type)
    {
        return; // TODO ???
    }

    EntityID BuildingManager::UnstashEntity(BuildingType type, glm::ivec2 newPos)
    {
        return 0; // TODO ???
    }

    float BuildingManager::GetMapGroundYAt(float worldX, float worldZ) const
    {
        Map* pMap = m_Manager->GetMap();
        return pMap ? pMap->GetGroundYAt(worldX, worldZ) : 0.0f;
    }

}
