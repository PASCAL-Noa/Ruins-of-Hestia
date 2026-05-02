#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "ErrorType.h"
#include "Object.h"
#include "RenderDevice.h"

namespace GPC
{
    class RenderCommands;

    typedef enum BufferUsageBits : int32_t {
        TRANSFER_SRC_BIT                          = VK_BUFFER_USAGE_TRANSFER_SRC_BIT                         ,
        TRANSFER_DST_BIT                          = VK_BUFFER_USAGE_TRANSFER_DST_BIT                         ,
        UNIFORM_TEXEL_BUFFER_BIT                  = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT                 ,
        STORAGE_TEXEL_BUFFER_BIT                  = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT                 ,
        UNIFORM_BUFFER_BIT                        = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT                       ,
        STORAGE_BUFFER_BIT                        = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT                       ,
        INDEX_BUFFER_BIT                          = VK_BUFFER_USAGE_INDEX_BUFFER_BIT                         ,
        VERTEX_BUFFER_BIT                         = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT                        ,
        INDIRECT_BUFFER_BIT                       = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT                      ,
        SHADER_DEVICE_ADDRESS_BIT                 = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT                ,
        VIDEO_DECODE_SRC_BIT_KHR                  = VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR                 ,
        VIDEO_DECODE_DST_BIT_KHR                  = VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR                 ,
        TRANSFORM_FEEDBACK_BUFFER_BIT_EXT         = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT        ,
        TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
        CONDITIONAL_RENDERING_BIT_EXT             = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT            ,
    } BufferUsageBits;
    typedef uint32_t BufferUsages;

    struct BufferInformation
    {
        BufferUsages            Usages;
        VkMemoryPropertyFlags   Flags;
        uint64_t                TotalSize;
    };

    GPC_INHERIT_OBJECT(Buffer)
    {

    public:

        // TEMPLATE TO PASS ElementType HERE
        Buffer() = default;
        ~Buffer() override = default;

        ErrorType Create(std::string_view name, BufferInformation const& buffer);
        void Destroy();

        VkBuffer        const&  GetBuffer() const;
        VkDeviceMemory  const&  GetBufferUploader() const;

        template<typename ElementType>
        ErrorType Map(ElementType** data, uint64_t offset, uint64_t count);
        void Unmap();

        ErrorType Copy(Buffer const& buffer);

    private:
        VkBuffer        m_Buffer            = { VK_NULL_HANDLE };
        VkDeviceMemory  m_BufferUploader    = { VK_NULL_HANDLE };

        uint64_t        m_Size              = { 0 };
        uint64_t        m_MappedSize        = { 0 };
        bool            m_IsUploaded        = { false };
    };

    template<typename ElementType>
    ErrorType Buffer::Map(ElementType** data, uint64_t offset, uint64_t count)
    {

        if (sizeof(ElementType) * count > m_Size)
            return ErrorType::VULKAN_BUFFER_CREATION;
        m_Size = sizeof(ElementType) * count;

        VkResult result = vkMapMemory(RenderDevice::GetDevice(), m_BufferUploader, offset, m_Size, 0, reinterpret_cast<void**>(data));
        return result == VK_SUCCESS ? ErrorType::SUCCESS : ErrorType::VULKAN_BUFFER_CREATION;

    }
} // GPC