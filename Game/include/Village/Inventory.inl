namespace GPC
{

    template<typename T>
    void Inventory::AddResource(uint32_t amount)
    {
        AddResource(T::ResourceId, amount);
    }

    template<typename T>
    bool Inventory::ConsumeResource(uint32_t amount)
    {
        auto resource = m_ResourceInventory.find(T::ResourceId);
        if (resource != m_ResourceInventory.end() && resource->second->GetCount() >= amount)
        {
            resource->second->Remove(amount);
            return true;
        }
        return false;
    }

    template<typename T>
    uint32_t Inventory::GetResourceCount() const
    {
        auto resource = m_ResourceInventory.find(T::ResourceId);
        if (resource != m_ResourceInventory.end())
        {
            return resource->second->GetCount();
        }
        return 0;
    }

    template<typename T>
    TextureID Inventory::GetResourceTexture() const
    {
        auto resource = m_ResourceInventory.find(T::ResourceId);
        if (resource != m_ResourceInventory.end())
        {
            return resource->second->GetTexture();
        }
        return 0;
    }

    template<template<ResourceTier> class T>
    void Inventory::RegisterTieredResources()
    {
        RegisterResource<T<ResourceTier::T1>>();
        RegisterResource<T<ResourceTier::T2>>();
        RegisterResource<T<ResourceTier::T3>>();
        RegisterResource<T<ResourceTier::T4>>();
        RegisterResource<T<ResourceTier::T5>>();
    }

    template<typename... Ts>
    void Inventory::RegisterResources()
    {
        (RegisterResource<Ts>(), ...);
    }

    template<typename T>
    void Inventory::RegisterResource()
    {
        T* newResource = new T();
        m_ResourceInventory[newResource->GetResourceId()] = newResource;
    }

}