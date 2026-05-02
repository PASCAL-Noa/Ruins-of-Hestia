#pragma once

#include <glm/vec2.hpp>

namespace GPC
{
    struct BuildingBehavior;

    enum class BuildingType : uint16_t
    {
        House   ,
        Farm    ,
        Forge   ,
        LifeTree,
        Hospital,
        Barrack ,
        Count
    };

    struct BuildingData
    {
        BuildingBehavior*   LinkedBehavior = nullptr;
        BuildingType        Type;
        glm::ivec2          Position;
        bool                IsStashed = false;
        bool                IsActive  = false;
    };
}
