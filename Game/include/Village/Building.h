#pragma once
#include <string>
#include <glm/vec2.hpp>

#include "Behavior.h"
#include "BuildingData.h"
#include "ECS_Defines.h"
#include "Scene.h"

namespace GPC
{
    struct Resource;
    struct BuildingBehavior;

    struct BuildingNeeds
    {
        std::string Texture;
        int32_t    Amount;
        uint32_t    ResourceId;
    };

    struct Building
    {

    public:
        Building(
            BuildingType type,
            const std::string& buildingIcon,
            const std::string& buildingName,
            const std::string& buildingDesc,
            const std::string& path,
            const std::string &highlightPath,
            glm::ivec2 size,
            float scale,
            bool UnlockByDefault,
            glm::vec2 offsetPlacement = glm::vec2(0.0f, 0.0f));
        virtual ~Building() = default;

        template<typename B_Behaviour>
        BuildingBehavior*                   Create(Scene* scene);

        [[nodiscard]] const std::string&    GetTexturePath() const;
        [[nodiscard]] const glm::ivec2&     GetSize() const;
        [[nodiscard]] const float&          GetScale() const;
        [[nodiscard]] EntityID              GetEntityID() const;
        void                                SetEntityID(EntityID entityID);

        template<typename R>
        requires(std::is_base_of_v<Resource, R>)
        void SetDependency(int32_t amount)
        {
            R resource;
            DependentTo = { resource.GetTexture(), amount, resource.GetResourceId() };
        }

        template<typename R>
        requires(std::is_base_of_v<Resource, R>)
        void AddUpgradeResource(uint32_t amount)
        {
            R resource;
            Upgrades.emplace_back(resource.GetTexture(), amount, resource.GetResourceId());
        }

        BuildingType            Type;

        std::string             Icon;
        std::string             Name;
        std::string             Description;

        BuildingNeeds           DependentTo;
        std::vector<BuildingNeeds>Upgrades;

        std::string             TexturePath;
        std::string             TextureHighlightPath;
        std::string             ActivePath;

        glm::ivec2              Size;
        glm::vec2               Offset;
        float                   Scale;

        bool                    Unlocked;

        float                   SpriteDepthFactor   = 0.78f;
        float                   SpriteLateralFactor = 0.05f;
    protected:
        BuildingBehavior*       m_Behavior{};

    private:

        EntityID                m_EntityID;
    };

    template<typename B_Behaviour>
    BuildingBehavior* Building::Create(Scene *scene)
    {
        auto b = scene->AddBehavior<B_Behaviour>(m_EntityID);
        m_Behavior = b;
        return b;
    }
}
