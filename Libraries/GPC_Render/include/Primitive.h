#pragma once
#include <glm/vec2.hpp>

#include "Geometry.h"
#include "Object.h"

namespace GPC
{
    struct GeometryData;

    GPC_INHERIT_OBJECT(Primitive)
    {

        public:
        static void CreateCube(glm::vec3 const& dimension, GeometryData* in); // TODO METTRE UN SHARED PTR
        static void CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount, GeometryData* in);
        static void CreatePlane(glm::vec2 const& dimension, uint32_t split, GeometryData* in);
        static void CreatePlaneDS(glm::vec2 const& dimension, uint32_t split, GeometryData* in);
        static void CreateRectangle(glm::vec2 const& dimension, GeometryData* in);
        static void CreateTristan(glm::vec2 const& dimension, GeometryData* in);

    };

} // GPC