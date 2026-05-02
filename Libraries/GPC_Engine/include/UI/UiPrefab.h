#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "ECS_Defines.h"

namespace GPC
{
    class Scene;
    class UiBuilder;
    struct UiCanvas;

    using UiPrefabFactoryFn = std::function<UiCanvas*(UiBuilder& builder, EntityID parent)>;

    class UiPrefabRegistry
    {
    public:
        static UiPrefabRegistry& Instance();

        void       Register(const std::string& name, UiPrefabFactoryFn factory);
        UiCanvas* Instantiate(const std::string& name, UiBuilder& builder, EntityID parent) const;
        bool       Has(const std::string& name) const;
        void       Clear();

    private:
        std::unordered_map<std::string, UiPrefabFactoryFn> m_Factories;
    };

    class UiPrefab
    {
    public:
        static UiCanvas* Instantiate(Scene* pScene, const std::string& name, EntityID parent = MAX_ENTITIES);
    };
}
