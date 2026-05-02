
#pragma once

#include <glm/glm.hpp>

#include "ComputeProgram.h"
#include "GraphicsProgram.h"

namespace GPC
{

    struct Batched3DObject
    {

        glm::vec4                    Position          = {};
        glm::quat                    Rotation          = {};
        glm::vec4                    Size              = {};
        Color                        Tint              = {};

        static VkVertexInputBindingDescription GetBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

    };

    inline VkVertexInputBindingDescription Batched3DObject::GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(Batched3DObject);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        return bindingDescription;
    }

    inline std::vector<VkVertexInputAttributeDescription> Batched3DObject::GetAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

        attributeDescriptions[0].binding = 1;
        attributeDescriptions[0].location = 3;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Batched3DObject, Position);

        attributeDescriptions[1].binding = 1;
        attributeDescriptions[1].location = 4;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Batched3DObject, Rotation);

        attributeDescriptions[2].binding = 1;
        attributeDescriptions[2].location = 5;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Batched3DObject, Size);

        attributeDescriptions[3].binding = 1;
        attributeDescriptions[3].location = 6;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Batched3DObject, Tint);

        return attributeDescriptions;
    }

    struct RenderObject3DBatched
    {
        virtual ~RenderObject3DBatched() = default;

        Geometry            const*    Mesh              = { nullptr };
        Material            const*    Material_          = { nullptr };
        GraphicsProgram     const*    GraphicProgram    = { nullptr };

        Batched3DObject*              BatchedObjects    = { nullptr };
        uint32_t                      EntityCount       = { 0 };
        bool                          UseShadow         = { false };

    };


}
