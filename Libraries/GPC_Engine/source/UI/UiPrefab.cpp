#include "UI/UiPrefab.h"

#include "UI/UiBuilder.h"
#include "UI/UiCanvas.h"

namespace GPC
{
    UiPrefabRegistry& UiPrefabRegistry::Instance()
    {
        static UiPrefabRegistry instance;
        return instance;
    }

    void UiPrefabRegistry::Register(const std::string& name, UiPrefabFactoryFn factory)
    {
        m_Factories[name] = std::move(factory);
    }

    bool UiPrefabRegistry::Has(const std::string& name) const
    {
        return m_Factories.find(name) != m_Factories.end();
    }

    UiCanvas* UiPrefabRegistry::Instantiate(const std::string& name, UiBuilder& builder, EntityID parent) const
    {
        const auto it = m_Factories.find(name);
        if (it == m_Factories.end()) return nullptr;
        return it->second(builder, parent);
    }

    void UiPrefabRegistry::Clear()
    {
        m_Factories.clear();
    }

    UiCanvas* UiPrefab::Instantiate(Scene* pScene, const std::string& name, EntityID parent)
    {
        if (!pScene) return nullptr;
        UiBuilder builder(pScene);
        return UiPrefabRegistry::Instance().Instantiate(name, builder, parent);
    }
}
