#pragma once

#include "Buffer.h"
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"
#include "Object.h"

namespace GPC
{

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normals;
        glm::vec2 TextureCoordinate;

        Vertex();
        Vertex(float x, float y, float z, float xN, float yN, float zN, float xT, float yT);

        static VkVertexInputBindingDescription GetBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };

    struct GeometryData
    {
        std::vector<Vertex>         Vertices    = {};
        std::vector<uint16_t>       Indices     = {};

        GeometryData() = default;
        GeometryData(uint32_t vertex_count, uint32_t indices_count);
    };

    GPC_INHERIT_OBJECT(Geometry)
    {

    public:
        Geometry() = default;
        ~Geometry() override = default;

        ErrorType   Create(GeometryData const* base);
        ErrorType   UpdateVertices(std::vector<Vertex> const& vertices);
        ErrorType   UpdateIndices(std::vector<uint16_t> const& indices);
        ErrorType   UpdateMeshData(std::vector<Vertex> const& vertices, std::vector<uint16_t> const& indices);
        void        Destroy();

        [[nodiscard]] Buffer const& GetVertexBuffer()const      { return m_VertexBuffer; };
        [[nodiscard]] Buffer const& GetIndexBuffer() const      { return m_IndexBuffer; };

        [[nodiscard]] uint32_t GetVerticesCount() const      { return m_VerticesCount; };
        [[nodiscard]] uint32_t GetIndexCount() const         { return m_IndicesCount; };

    private:

        uint32_t m_VerticesCount = 0;
        uint32_t m_IndicesCount = 0;

        Buffer m_StagingBuffer;
        Buffer m_VertexBuffer;
        Buffer m_IndexBuffer;

    };
} // GPC