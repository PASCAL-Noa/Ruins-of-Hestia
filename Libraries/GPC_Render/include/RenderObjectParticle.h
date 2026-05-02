#pragma once

#include "ComputeProgram.h"
#include "Geometry.h"
#include "Material.h"

namespace GPC
{

    struct RenderObjectParticle
    {
        virtual ~RenderObjectParticle() = default;

        Geometry            const*      Mesh                = { nullptr };
        glm::mat4           const*      Transform           = { nullptr };
        Material            const*      Material_            = { nullptr };
        ComputeProgram      const*      ComputeProgram_      = { nullptr };

    };

}
