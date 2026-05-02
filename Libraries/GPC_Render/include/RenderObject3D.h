#pragma once
#include <glm/glm.hpp>

#include "GraphicsProgram.h"
#include "Object.h"

#include "Material.h"

namespace GPC
{
    class Geometry;

    struct Object3DConstants {
        uint32_t ObjectID       = {0};
    };

    struct ShadowPushConstant
    {
        uint32_t objectID;
        uint32_t lightBaseIndex;
        uint32_t textureArray;
        uint32_t textureId;
    };

    struct ShadowPointPushConstant
    {
        uint32_t objectID;
        uint32_t lightBaseIndex;
        float    farPlane;
        float    _pad;
        glm::vec4 lightWorldPos;
    };

    struct RenderObject3D
    {
        virtual ~RenderObject3D() = default;

        Geometry            const*    Mesh              = { nullptr };
        glm::mat4           const*    Transform         = { nullptr };
        Material            const*    Material_          = { nullptr };
        GraphicsProgram     const*    GraphicProgram    = { nullptr };
        bool                          UseShadow         = { false };

    };
} // GPC