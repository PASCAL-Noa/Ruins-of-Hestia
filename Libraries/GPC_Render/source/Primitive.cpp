#include "Primitive.h"
#include "Geometry.h"


namespace GPC
{

    void Primitive::CreateCube(glm::vec3 const& dimension, GeometryData* in)
    {

        glm::vec3 s = dimension * 0.5f;

        // Create the vertices.
        in->Vertices = {
            // FRONT FACE (normal: +Z)
            {-s.x, -s.y, +s.z,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f},
            {+s.x, -s.y, +s.z,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f},
            {+s.x, +s.y, +s.z,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f},
            {-s.x, +s.y, +s.z,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f},

            // BACK FACE (normal: -Z)
            {+s.x, -s.y, -s.z,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f},
            {-s.x, -s.y, -s.z,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f},
            {-s.x, +s.y, -s.z,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f},
            {+s.x, +s.y, -s.z,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f},

            // TOP FACE (normal: +Y)
            {-s.x, +s.y, +s.z,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f},
            {+s.x, +s.y, +s.z,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f},
            {+s.x, +s.y, -s.z,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f},
            {-s.x, +s.y, -s.z,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f},

            // BOTTOM FACE (normal: -Y)
            {-s.x, -s.y, -s.z,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f},
            {+s.x, -s.y, -s.z,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f},
            {+s.x, -s.y, +s.z,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f},
            {-s.x, -s.y, +s.z,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f},

            // LEFT FACE (normal: -X)
            {-s.x, -s.y, -s.z, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f},
            {-s.x, -s.y, +s.z, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f},
            {-s.x, +s.y, +s.z, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f},
            {-s.x, +s.y, -s.z, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f},

            // RIGHT FACE (normal: +X)
            {+s.x, -s.y, +s.z,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f},
            {+s.x, -s.y, -s.z,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f},
            {+s.x, +s.y, -s.z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f},
            {+s.x, +s.y, +s.z,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f},
        };

        // Indices (all faces wound counter-clockwise from outside)
        in->Indices = {
            // Front face
            0,  1,  2,
            0,  2,  3,

            // Back face
            4,  5,  6,
            4,  6,  7,

            // Top face
            8,  9,  10,
            8,  10, 11,

            // Bottom face
            12, 13, 14,
            12, 14, 15,

            // Left face
            16, 17, 18,
            16, 18, 19,

            // Right face
            20, 21, 22,
            20, 22, 23,
        };

    }

    void Primitive::CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount, GeometryData *in)
    {
        in->Vertices.clear();
        in->Indices.clear();

        // -------------------------------------------------------
        // Pôle Nord
        // -------------------------------------------------------
        in->Vertices.push_back({
            0.0f, +radius, 0.0f,   // Position
            0.0f,  +1.0f,  0.0f,   // Normal
            0.5f,   0.0f          // UV
        });

        // -------------------------------------------------------
        // Anneaux intermédiaires (stacks)
        // -------------------------------------------------------
        float phiStep   = glm::pi<float>()        / stackCount;   // 0 → π   (haut → bas)
        float thetaStep = 2.0f * glm::pi<float>() / sliceCount;  // 0 → 2π  (tour complet)

        for (uint32_t i = 1; i <= stackCount - 1; ++i)
        {
            float phi = i * phiStep;   // angle vertical

            for (uint32_t j = 0; j <= sliceCount; ++j)
            {
                float theta = j * thetaStep;   // angle horizontal

                glm::vec3 pos = {
                    radius * sinf(phi) * cosf(theta),
                    radius * cosf(phi),
                    radius * sinf(phi) * sinf(theta)
                };

                glm::vec3 normal = glm::normalize(pos);

                glm::vec2 uv = {
                    theta / (2.0f * glm::pi<float>()),   // U : 0 → 1
                    phi   /         glm::pi<float>()     // V : 0 → 1
                };

                in->Vertices.push_back({
                    pos.x,    pos.y,    pos.z,
                    normal.x, normal.y, normal.z,
                    uv.x,     uv.y
                });
            }
        }

        // -------------------------------------------------------
        // Pôle Sud
        // -------------------------------------------------------
        in->Vertices.push_back({
             0.0f, -radius,  0.0f,   // Position
             0.0f,  -1.0f,   0.0f,   // Normal
             0.5f,   1.0f           // UV
        });

        // -------------------------------------------------------
        // Indices — Capuchon Nord (1er stack)
        // -------------------------------------------------------
        for (uint32_t i = 1; i <= sliceCount; ++i)
        {
            in->Indices.push_back(0);
            in->Indices.push_back(i + 1);
            in->Indices.push_back(i);
        }

        // -------------------------------------------------------
        // Indices — Anneaux intermédiaires
        // -------------------------------------------------------
        uint32_t baseIndex    = 1;                        // 1er vertex du 1er anneau
        uint32_t ringVertCount = sliceCount + 1;          // vertices par anneau

        for (uint32_t i = 0; i < stackCount - 2; ++i)
        {
            for (uint32_t j = 0; j < sliceCount; ++j)
            {
                // Triangle 1
                in->Indices.push_back(baseIndex + i       * ringVertCount + j);
                in->Indices.push_back(baseIndex + i       * ringVertCount + j + 1);
                in->Indices.push_back(baseIndex + (i + 1) * ringVertCount + j);

                // Triangle 2
                in->Indices.push_back(baseIndex + (i + 1) * ringVertCount + j);
                in->Indices.push_back(baseIndex + i       * ringVertCount + j + 1);
                in->Indices.push_back(baseIndex + (i + 1) * ringVertCount + j + 1);
            }
        }

        // -------------------------------------------------------
        // Indices — Capuchon Sud (dernier stack)
        // -------------------------------------------------------
        uint32_t southPoleIndex = (uint32_t)in->Vertices.size() - 1;
        baseIndex = southPoleIndex - ringVertCount;

        for (uint32_t i = 0; i < sliceCount; ++i)
        {
            in->Indices.push_back(southPoleIndex);
            in->Indices.push_back(baseIndex + i);
            in->Indices.push_back(baseIndex + i + 1);
        }
    }

    void Primitive::CreatePlane(glm::vec2 const &dimension, uint32_t split, GeometryData *in)
    {

        double partSizeX = dimension.x / (double)split;
        double partSizeY = dimension.y / (double)split;

        for (int32_t x = 0; x < split; ++x) {
            for (int32_t y = 0; y < split; ++y) {

                // Centrage du plan autour de l'origine
                double baseX = -dimension.x * 0.5f + (double)x * partSizeX;
                double baseY = -dimension.y * 0.5f + (double)y * partSizeY;

                double baseX1 = -dimension.x * 0.5f + (double)(x+1) * partSizeX;
                double baseY1 = -dimension.y * 0.5f + (double)(y+1) * partSizeY;

                // TOP FACE (normal: +Y)
                int32_t index = (int32_t)in->Vertices.size();
                in->Vertices.emplace_back(baseX,            0.0f,   baseY1,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f);
                in->Vertices.emplace_back(baseX1, 0.0f,     baseY1,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f);
                in->Vertices.emplace_back(baseX1, 0.0f, baseY,             0.0f, 1.0f, 0.0f,  1.0f, 0.0f);
                in->Vertices.emplace_back(baseX,            0.0f, baseY,             0.0f, 1.0f, 0.0f,  0.0f, 0.0f);

                in->Indices.push_back(index);
                in->Indices.push_back(index + 1);
                in->Indices.push_back(index + 2);
                in->Indices.push_back(index);
                in->Indices.push_back(index + 2);
                in->Indices.push_back(index + 3);
            }
        }

    }

    void Primitive::CreatePlaneDS(glm::vec2 const &dimension, uint32_t split, GeometryData *in) {
        double partSizeX = dimension.x / (double)split;
        double partSizeY = dimension.y / (double)split;

        for (int32_t x = 0; x < split; ++x) {
            for (int32_t y = 0; y < split; ++y) {

                // Centrage du plan autour de l'origine
                double baseX = -dimension.x * 0.5f + (double)x * partSizeX;
                double baseY = -dimension.y * 0.5f + (double)y * partSizeY;

                double baseX1 = -dimension.x * 0.5f + (double)(x+1) * partSizeX;
                double baseY1 = -dimension.y * 0.5f + (double)(y+1) * partSizeY;

                // TOP FACE (normal: +Y)
                int32_t index = (int32_t)in->Vertices.size();
                in->Vertices.emplace_back(baseX,            0.0f,   baseY1,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f);
                in->Vertices.emplace_back(baseX1, 0.0f,     baseY1,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f);
                in->Vertices.emplace_back(baseX1, 0.0f, baseY,             0.0f, 1.0f, 0.0f,  1.0f, 0.0f);
                in->Vertices.emplace_back(baseX,            0.0f, baseY,             0.0f, 1.0f, 0.0f,  0.0f, 0.0f);

                in->Indices.push_back(index);
                in->Indices.push_back(index + 1);
                in->Indices.push_back(index + 2);
                in->Indices.push_back(index);
                in->Indices.push_back(index + 2);
                in->Indices.push_back(index + 3);
                in->Indices.push_back(index + 1);
                in->Indices.push_back(index);
                in->Indices.push_back(index + 2);
                in->Indices.push_back(index + 2);
                in->Indices.push_back(index);
                in->Indices.push_back(index + 3);
            }
        }
    }

    void Primitive::CreateRectangle(glm::vec2 const &dimension, GeometryData *in)
    {

        glm::vec2 s = dimension * 0.5f;

        // TODO METTRE UN CONTAINER QUI PRE ALLOU PLUTOT QUE DE FAIRE PUSH BACK
        in->Vertices.emplace_back(-s.x,         s.y, 0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f);
        in->Vertices.emplace_back(s.x,          s.y, 0.0f, 0.0f, 1.0f, 0.0f,  1.0f, 1.0f);
        in->Vertices.emplace_back(s.x,          -s.y, 0.0f, 0.0f, 1.0f, 0.0f,  1.0f, 0.0f);
        in->Vertices.emplace_back(-s.x,         -s.y, 0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f);

        in->Indices.push_back(0);
        in->Indices.push_back(1);
        in->Indices.push_back(2);
        in->Indices.push_back(0);
        in->Indices.push_back(2);
        in->Indices.push_back(3);

    }

    void Primitive::CreateTristan(glm::vec2 const &dimension, GeometryData *in)
    {
        glm::vec2 s = dimension * 0.5f;

        // TODO METTRE UN CONTAINER QUI PRE ALLOU PLUTOT QUE DE FAIRE PUSH BACK
        in->Vertices.emplace_back(-s.x,         s.y, 0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f);
        in->Vertices.emplace_back(s.x,          s.y, 0.0f, 0.0f, 1.0f, 0.0f,  1.0f, 1.0f);
        in->Vertices.emplace_back(s.x,          -s.y, 0.0f, 0.0f, 1.0f, 0.0f,  1.0f, 0.0f);

        in->Indices.push_back(0);
        in->Indices.push_back(1);
        in->Indices.push_back(2);
    }

} // GPC