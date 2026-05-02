#include "Geometry.h"

#include <algorithm>
#include <cstring>

#include "IProgram.h"

namespace GPC
{
    template <typename T>
    ErrorType UploadDataToBuffer(Buffer& target, const char* stagingName, const std::vector<T>& data)
    {
        if (data.empty())
            return ErrorType::SUCCESS;

        const uint64_t dataSize = sizeof(T) * data.size();

        Buffer stagingBuffer;
        const ErrorType stagingCreate = stagingBuffer.Create(stagingName, {
            .Usages = BufferUsageBits::TRANSFER_SRC_BIT,
            .Flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            .TotalSize = dataSize
        });
        if (GPC_FAILED(stagingCreate))
            return stagingCreate;

        T* mappedData = nullptr;
        const ErrorType mapResult = stagingBuffer.Map<T>(&mappedData, 0, data.size());
        if (GPC_FAILED(mapResult))
        {
            stagingBuffer.Destroy();
            return mapResult;
        }

        memcpy(mappedData, data.data(), dataSize);
        stagingBuffer.Unmap();

        const ErrorType copyResult = target.Copy(stagingBuffer);
        stagingBuffer.Destroy();
        return copyResult;
    }

    Vertex::Vertex(): Position(0, 0, 0), Normals(0, 0, 0), TextureCoordinate(0, 0) {}

    Vertex::Vertex(float x, float y, float z, float xN, float yN, float zN, float xT, float yT)
        : Position(x, y, z), Normals(xN, yN, zN), TextureCoordinate(xT, yT) {}

    VkVertexInputBindingDescription Vertex::GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

        // layout(location(#location) = 0) in vec2(#format) inPosition;
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, Position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, Normals);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, TextureCoordinate);

        return attributeDescriptions;
    }

    GeometryData::GeometryData(uint32_t vertex_count, uint32_t indices_count) {
        Vertices.resize(vertex_count);
        Indices.resize(indices_count);
    }

    ErrorType Geometry::Create(GeometryData const* base)
    {
        if (!base)
            return ErrorType::VULKAN_BUFFER_CREATION;

        m_IndicesCount = static_cast<uint32_t>(base->Indices.size());
        m_VerticesCount = static_cast<uint32_t>(base->Vertices.size());

        const uint64_t vertexSize = sizeof(Vertex) * m_VerticesCount;
        const uint64_t indexSize = sizeof(uint16_t) * m_IndicesCount;

        const ErrorType stagingCreate = m_StagingBuffer.Create("STAGING_BUFFER", {
            .Usages = BufferUsageBits::TRANSFER_SRC_BIT,
            .Flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            .TotalSize = std::max(vertexSize, indexSize)
        });
        if (GPC_FAILED(stagingCreate))
            return stagingCreate;

        const ErrorType vertexCreate = m_VertexBuffer.Create("VERTEX BUFFER", {
            .Usages = BufferUsageBits::TRANSFER_DST_BIT | BufferUsageBits::VERTEX_BUFFER_BIT,
            .Flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .TotalSize = vertexSize
        });
        if (GPC_FAILED(vertexCreate))
            return vertexCreate;

        const ErrorType indexCreate = m_IndexBuffer.Create("INDEX_BUFFER", {
            .Usages = BufferUsageBits::TRANSFER_DST_BIT | BufferUsageBits::INDEX_BUFFER_BIT,
            .Flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .TotalSize = indexSize
        });
        if (GPC_FAILED(indexCreate))
            return indexCreate;

        if (m_VerticesCount > 0)
        {
            Vertex* vertex = nullptr;
            const ErrorType mapVertices = m_StagingBuffer.Map<Vertex>(&vertex, 0, m_VerticesCount);
            if (GPC_FAILED(mapVertices))
                return mapVertices;

            memcpy(vertex, base->Vertices.data(), vertexSize);
            m_StagingBuffer.Unmap();
            m_VertexBuffer.Copy(m_StagingBuffer);
        }

        if (m_IndicesCount > 0)
        {
            uint16_t* indices = nullptr;
            const ErrorType mapIndices = m_StagingBuffer.Map<uint16_t>(&indices, 0, m_IndicesCount);
            if (GPC_FAILED(mapIndices))
                return mapIndices;

            memcpy(indices, base->Indices.data(), indexSize);
            m_StagingBuffer.Unmap();
            m_IndexBuffer.Copy(m_StagingBuffer);
        }

        m_StagingBuffer.Destroy();
        return ErrorType::SUCCESS;
    }

    ErrorType Geometry::UpdateVertices(std::vector<Vertex> const& vertices)
    {
        if (vertices.empty())
            return ErrorType::SUCCESS;

        if (vertices.size() != m_VerticesCount)
        {
            m_VertexBuffer.Destroy();
            m_VerticesCount = static_cast<uint32_t>(vertices.size());

            const ErrorType recreateResult = m_VertexBuffer.Create("VERTEX BUFFER", {
                .Usages = BufferUsageBits::TRANSFER_DST_BIT | BufferUsageBits::VERTEX_BUFFER_BIT,
                .Flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                .TotalSize = sizeof(Vertex) * m_VerticesCount
            });
            if (GPC_FAILED(recreateResult))
                return recreateResult;
        }

        return UploadDataToBuffer(m_VertexBuffer, "VERTEX_STAGING_BUFFER", vertices);
    }

    ErrorType Geometry::UpdateIndices(std::vector<uint16_t> const& indices)
    {
        if (indices.empty())
            return ErrorType::SUCCESS;

        if (indices.size() != m_IndicesCount)
        {
            m_IndexBuffer.Destroy();
            m_IndicesCount = static_cast<uint32_t>(indices.size());

            const ErrorType recreateResult = m_IndexBuffer.Create("INDEX_BUFFER", {
                .Usages = BufferUsageBits::TRANSFER_DST_BIT | BufferUsageBits::INDEX_BUFFER_BIT,
                .Flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                .TotalSize = sizeof(uint16_t) * m_IndicesCount
            });
            if (GPC_FAILED(recreateResult))
                return recreateResult;
        }

        return UploadDataToBuffer(m_IndexBuffer, "INDEX_STAGING_BUFFER", indices);
    }

    ErrorType Geometry::UpdateMeshData(std::vector<Vertex> const& vertices, std::vector<uint16_t> const& indices)
    {
        const ErrorType vertexResult = UpdateVertices(vertices);
        if (GPC_FAILED(vertexResult))
            return vertexResult;

        return UpdateIndices(indices);
    }

    void Geometry::Destroy()
    {
        m_VertexBuffer.Destroy();
        m_IndexBuffer.Destroy();
    }
} // GPC