#pragma once

#include "Color.h"
#include "glm/glm.hpp"
#include "Object.h"

namespace GPC
{

#pragma pack(push, 1)
    struct Material
    {

        using TextureID     = uint32_t;
        using TextureOffset = uint32_t;

        Color           Tint            = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        float           Metalic         {0.30f};
        float           Roughness       {0.30f};
        float           AO              {1.0f};
        float           EmissiveStrength{0.0f};
        TextureID       TextureArray    {0};
        TextureOffset   Diffuse         {0};
        TextureOffset   NormalMap       {0};
        TextureOffset   HeightMap       {1};

    };
#pragma pack(pop)

} // GPC