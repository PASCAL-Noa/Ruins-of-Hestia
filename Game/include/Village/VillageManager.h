#pragma once
#include <unordered_map>
#include <cstdint>

#include "AudioSystem.h"
#include "Buildings/BuildingBehavior.h"
#include "BuildingData.h"
#include "Inventory.h"
#include "VillageController.h"
#include "Map/Editor.h"

namespace GPC
{

    using BuildingEventCallback = std::function<BuildingBehavior*(BuildingType, glm::ivec2)>;
    using BuildingStashedCallback = std::function<void(glm::ivec2, BuildingType)>;
    using BuildingUnstashedCallback = std::function<void(BuildingBehavior*, glm::ivec2)>;

    class Map;

    class VillageManager
    {
    public:
        VillageManager(Scene* scene);
        ~VillageManager();

        struct BuildingCost
        {
            ResourceID                  Id;
            uint32_t                    Amount;
        };

        struct BuildingTemplate
        {
            std::string                 Path;
            glm::ivec2                  Size;
            float                       Scale;
            Building*                   Infos;
        };

        [[nodiscard]] const BuildingTemplate&               GetBuildingTemplate(BuildingType type) const;

        void                                                Update();
        void                                                AdvanceTime();
        int                                                 GetExpeditionCount() const;

        bool                                                LoadVillageData();
        void                                                SaveVillageData();
        void                                                Reset();

        bool                                                CanAfford(BuildingType type) const;
        void                                                PayForBuilding(BuildingType type);

        std::vector<BuildingData>&                          GetBuildingsData();
        size_t                                              GetBuildingsCount() const;
        BuildingBehavior*                                   AddBuilding(BuildingType type, glm::ivec2 position);
        void                                                UpdateBuildingPosition(size_t index, glm::ivec2 newPosition);

        BuildingEventCallback                               OnBuildingAddedCallback = nullptr;
        BuildingStashedCallback                             OnBuildingStashedCallback = nullptr;
        BuildingUnstashedCallback                           OnBuildingUnstashedCallback = nullptr;

        void                                                StashBuildingAt(glm::ivec2 position);
        bool                                                UnstashBuilding(BuildingType type, glm::ivec2 newPosition);
        int                                                 GetStashedCount(BuildingType type) const;

        void                                                SetAllBuildingsActive(bool active);
        void                                                SyncBuildingActiveState();
        void                                                RebuildBehaviorRegistry();
        void                                                StaggerSwapToActiveTextures(float totalDuration);
        void                                                StaggerSwapToInactiveTextures(float totalDuration);

        Inventory&                                          GetInventory() { return m_Inventory; }

        Map*                                                GetMap() const { return mp_Map; }
        void                                                SetMap(Map* pMap) { mp_Map = pMap; }
        [[nodiscard]] Scene*                                GetScene() const { return mp_Scene; }
        void                                                SetVillageController(VillageController* pController) { mp_Controller = pController; }
        VillageController*                                  GetVillageController() { return mp_Controller; }

        void                                                StartPlacement(BuildingType type);
        void                                                UpdatePlacement(Scene* scene, VillageController* controller, float dt);

        bool                                                IsPlacingNew() const { return m_IsPlacingNew; }
        BuildingType                                        GetPlacementType() const { return m_PlacementType; }
        std::vector<AudioSource*>&                          GetAudioSources();
        std::vector<BuildingBehavior*> &                    GetBuildingBehavior() { return m_BuildingBehaviors; };

        void                                                GiveDefaultEquipments();

    private:
        std::vector<BuildingBehavior*>                      m_BuildingBehaviors;
        std::vector<BuildingData>                           m_BuildingsData;
        Inventory                                           m_Inventory;
        Scene*                                              mp_Scene;
        int                                                 m_ExpeditionCount;

        void                                                LoadDefaultVillage();
        void                                                GiveDefaultResource();
        void                                                LoadBuildingsFromTree(const SerialTree& tree);

        std::unordered_map<BuildingType, BuildingTemplate>  m_BuildingDatabase;
        void                                                BuildCatalogue();

        std::unordered_map<BuildingType, int>               m_StashedCounts;
        void                                                RecalculateStashCache();
        void                                                HandleInteract(Scene* scene) const;

        bool                                                m_IsPlacingNew = false;
        BuildingType                                        m_PlacementType;
        float                                               m_PlacementTimer = 0.0f;

        Map*                                                mp_Map = nullptr;
        VillageController*                                  mp_Controller = nullptr;
        std::vector<AudioSource*>                           mp_VillageSFXList;

    };
}

#include "VillageManager.inl"