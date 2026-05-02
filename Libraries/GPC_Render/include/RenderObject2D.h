#pragma once
#include <glm/vec2.hpp>

#include "Color.h"
#include "Material.h"
#include "Object.h"
#include "Texture.h"
#include "Font.h"

namespace GPC
{

    struct Instance2D
    {
        Color       Tint                = Color(0xffffffff);
        glm::vec2   Position            = {0, 0};
        glm::vec2   Size                = {100, 100};
        uint32_t    TextureIndex        = {0};
        uint32_t    TextureLayerOffset  = {0};
        float       Rotation            = 0.0f;

        static VkVertexInputBindingDescription GetBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };

    GPC_INHERIT_OBJECT( RenderObject2D )
    {
    public:
        RenderObject2D() = default;
        ~RenderObject2D() override = default;

        Color                   Tint                = Color(255, 255, 255, 255);
        glm::vec2               Position            = glm::vec2(0.0f);
        glm::vec2               Size                = glm::vec2(32.0f);
        int32_t                 ZIndex              = { 0 };
        float                   Rotation            = 0.0f;
        Material    const*      Material_            = { nullptr };
        Texture     const*      Texture_             = { nullptr };

    };

    GPC_INHERIT_OBJECT( RenderText2D )
    {
        public:
        RenderText2D() = default;
        ~RenderText2D() override = default;

        std::string             Text                = "Default";
        Color                   Tint                = Color(255, 255, 255, 255);
        glm::vec2               Position            = glm::vec2(0.0f);
        float                   Scale               = 1.0f;
        int32_t                 ZIndex              = { 0 };
        Font    const*          Font_                = { nullptr };

    };

}
