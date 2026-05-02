#include "Village/Building.h"

namespace GPC
{

    Building::Building(
        BuildingType type,
        const std::string &buildingIcon,
        const std::string &buildingName,
        const std::string &buildingDesc,
        const std::string &path,
        const std::string &highlightPath,
        glm::ivec2 size,
        float scale,
        bool UnlockedByDefault,
        glm::vec2 offsetPlacement)
        :   Icon(buildingIcon), Name(buildingName), Description(buildingDesc),
            TexturePath(path), TextureHighlightPath(highlightPath),
            Size(size), Scale(scale), Unlocked(UnlockedByDefault), m_EntityID(0),
            Type(type), Offset(offsetPlacement)
            {}

    const glm::ivec2& Building::GetSize() const
    {
        return Size;
    }

    const float & Building::GetScale() const
    {
        return Scale;
    }

    const std::string& Building::GetTexturePath() const
    {
        return TexturePath;
    }

    EntityID Building::GetEntityID() const
    {
        return m_EntityID;
    }

    void Building::SetEntityID(EntityID entityID)
    {
        m_EntityID = entityID;
    }
}