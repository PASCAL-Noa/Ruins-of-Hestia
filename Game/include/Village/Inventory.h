
#pragma once

#include "GPC_Framework.h"

#include "Resource.h"
#include "Expedition/Equipment.h"
#include "UI/UiSignal.h"

namespace GPC
{
    struct ResourceSaveData
    {
        uint32_t ID;
        uint32_t Count;
    };

    constexpr uint64_t MAX_EQUIPMENTS = 64;

    using Resources = std::unordered_map<uint32_t, Resource*>;
    using Equipments = std::unordered_map<uint32_t, Equipment*>;

    class Inventory
    {
    public:

        ~Inventory();
        void Initialize(bool fillDebugStock = true);

        UiSignal<ResourceID, uint32_t>                      OnResourceAdded;

        void                                                AddResource(ResourceID id, uint32_t amount);
        template<typename T> void                           AddResource(uint32_t amount);
        template<typename T> bool                           ConsumeResource(uint32_t amount);
        template<typename T> uint32_t                       GetResourceCount() const;
        template<typename T> TextureID                      GetResourceTexture() const;
        bool                                                HasResourceAmount(ResourceID id, uint32_t amount) const;
        bool                                                ConsumeResource(ResourceID id, uint32_t amount);

        void AddAllFromInventory(Inventory& other);

        void AddEquipment(Equipment* equipment);
        bool HasEquipment(EquipmentID id) const;
        Equipment* GetEquipment(EquipmentID id);
        [[nodiscard]] const Equipments& GetEquipments() const { return m_EquipmentInventory; }

        void Clear();
        void LosePercent(float percent);

        void SaveToFile(SerialTree& tree);
        void LoadFromFile(const SerialTree& tree);

    private:
        Resources                                           m_ResourceInventory{};
        Equipments                                          m_EquipmentInventory{};

        template<typename T> void                           RegisterResource();

        template<template<ResourceTier> class T> void       RegisterTieredResources();
        template<typename... Ts> void                       RegisterResources();


    };

} // GPC

#include "Village/Inventory.inl"
