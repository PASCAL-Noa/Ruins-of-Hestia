#pragma once
#include "Behavior.h"
#include "../BuildingBehavior.h"
#include "UI/UiReactive.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace GPC
{
    struct UiCanvas;
    struct UiSprite;
    struct UiText;
    class VillageManager;
    class Scene;
    class Equipment;

    class ForgeBehavior : public BuildingBehavior
    {
    public:
        INHERIT_SPECIFIC_BEHAVIOR_CONSTRUCTOR(ForgeBehavior, BuildingBehavior);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void OnInteract() override;
        void OnNextCycle() override;
        void CloseOpened() override;

    private:
        void CreateBaseForgeUI(const BehaviorCreateContext *pCtx);
        void RefreshInventoryUI();
        void BuildEquipDetailPanel(int equipIndex);
        void OnSocketClicked(int socketIndex);

        std::string GetEquipmentName(Equipment* pEquip) const;
        std::string GetEquipmentTexture(Equipment* pEquip) const;

        VillageManager* m_VillageManagerCache = nullptr;
        Scene* mp_Scene = nullptr;

        UiCanvas* m_ForgeCanvas = nullptr;
        UiCanvas* m_EmptyCanvas = nullptr;

        UiSprite* m_EquipSprites[6] = { nullptr };
        Equipment* m_MappedEquips[6] = { nullptr };

        UiCanvas* m_DetailCanvas = nullptr;
        UiText* m_DetailTitle = nullptr;
        UiText* m_StatNamesText[6] = { nullptr };
        UiText* m_StatValuesText[6] = { nullptr };
        UiSprite* m_SocketSprites[6] = { nullptr };
        std::string m_MappedStats[6] = { "" };

        Equipment* m_CurrentEquip = nullptr;
        std::unordered_map<std::string, int> m_PendingGems;

        UiReactive<int> m_GemCounts[6] = {
            UiReactive<int>(0), UiReactive<int>(0), UiReactive<int>(0),
            UiReactive<int>(0), UiReactive<int>(0), UiReactive<int>(0)
        };
    };
}