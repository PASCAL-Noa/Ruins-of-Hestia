#include "Village/Inventory.h"

#include <ranges>

#include "SerialTree.h"
#include "Village/Save.h"

namespace GPC
{

    Inventory::~Inventory()
    {
        for (auto& resources : m_EquipmentInventory)
        {
            delete resources.second;
        }
        m_EquipmentInventory.clear();

        for (auto &resource: m_ResourceInventory | std::views::values)
        {
            delete resource;
        }
        m_ResourceInventory.clear();
    }

    void Inventory::Initialize(bool fillDebugStock)
    {
        RegisterTieredResources<Wood>();
        RegisterTieredResources<Stone>();
        RegisterTieredResources<Metal>();
        RegisterTieredResources<Fiber>();

        RegisterResources<
            Food,
            Villager,
            FragmentCard,
            PurpleGemme,
            RedGemme,
            BlueGemme,
            GreenGemme,
            YellowGemme,
            OrangeGemme
        >();
    }

    bool Inventory::HasResourceAmount(ResourceID id, uint32_t amount) const
    {
        auto resource = m_ResourceInventory.find(id);
        if (resource != m_ResourceInventory.end())
        {
            return resource->second->Has(amount);
        }
        return false;
    }

    bool Inventory::ConsumeResource(ResourceID id, uint32_t amount)
    {
        auto resource = m_ResourceInventory.find(id);
        if (resource != m_ResourceInventory.end() && resource->second->Has(amount))
        {
            resource->second->Remove(amount);
            return true;
        }
        return false;
    }

    void Inventory::AddAllFromInventory(Inventory &other)
    {
        for (auto resources : other.m_ResourceInventory) {
            AddResource(resources.first, resources.second->GetCount());
        }

        for (auto& [id, pEq] : other.m_EquipmentInventory)
        {
            if (pEq != nullptr)
            {
                AddEquipment(pEq);
            }
        }
        other.m_EquipmentInventory.clear();
    }

    void Inventory::AddEquipment(Equipment *equipment)
    {
        m_EquipmentInventory.emplace(equipment->GetId(), equipment);
    }

    bool Inventory::HasEquipment(EquipmentID id) const
    {
        return m_EquipmentInventory.contains(id);
    }

    Equipment * Inventory::GetEquipment(EquipmentID id)
    {
        auto equipment = m_EquipmentInventory.find(id);
        if (equipment != m_EquipmentInventory.end())
        {
            return equipment->second;
        }
        return nullptr;
    }

    void Inventory::Clear()
    {
        m_ResourceInventory.clear();
        Initialize();
    }

    void Inventory::LosePercent(float percent)
    {
        for (auto it : m_ResourceInventory) {
            it.second->Remove(it.second->GetCount() * percent);
        }
    }

    void Inventory::SaveToFile(SerialTree& tree)
    {
        std::vector<ResourceSaveData> activeResources;
        activeResources.resize(m_ResourceInventory.size());

        uint32_t index = 0;
        for (const auto& [id, resource] : m_ResourceInventory)
        {
            activeResources[index] = {id, resource->GetCount()};
            index++;
        }

        if (!activeResources.empty())
        {
            tree[Saves::KEY_INVENTORY].Set(activeResources.data(), activeResources.size());
        }

    }

    void Inventory::LoadFromFile(const SerialTree& tree)
    {

        Clear();

        if (!tree.HasKey(Saves::KEY_INVENTORY))
            return;

        const SerialTree& inventoryNode = tree[Saves::KEY_INVENTORY];
        uint64_t count = inventoryNode.Count<ResourceSaveData>();
        ResourceSaveData* pSavedResources = inventoryNode.Get<ResourceSaveData>();
        for (uint64_t i = 0; i < count; ++i)
        {
            ResourceSaveData savedData;
            memcpy(&savedData, &pSavedResources[i], sizeof(ResourceSaveData));
            auto it = m_ResourceInventory.find(savedData.ID);
            if (it != m_ResourceInventory.end())
            {
                it->second->Add(savedData.Count);
            }
        }

    }

    void Inventory::AddResource(ResourceID id, uint32_t amount)
    {
        auto resource = m_ResourceInventory.find(id);
        if (resource != m_ResourceInventory.end())
        {
            resource->second->Add(amount);
            OnResourceAdded.Emit(id, amount);
        }
    }
} // GPC