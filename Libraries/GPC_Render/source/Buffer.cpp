#include "Buffer.h"

#include "RenderCommands.h"
#include "RenderDevice.h"

namespace GPC
{

    ErrorType Buffer::Create(std::string_view name, BufferInformation const& buffer)
    {

        if (buffer.Usages < 1)
            return ErrorType::VULKAN_BUFFER_CREATION;

        m_Size = buffer.TotalSize;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size         = buffer.TotalSize;
        bufferInfo.usage        = buffer.Usages;
        bufferInfo.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(RenderDevice::GetDevice(), &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS) {
            return ErrorType::VULKAN_BUFFER_CREATION;
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(RenderDevice::GetDevice(), m_Buffer, &memRequirements);

        RenderDevice::SetDebugName(VK_OBJECT_TYPE_BUFFER, name, reinterpret_cast<uint64_t>(m_Buffer));

        VkMemoryAllocateInfo allocInfo{}; // TODO Add sub allocation to not reach max
        allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize    = memRequirements.size;
        allocInfo.memoryTypeIndex   = RenderDevice::FindMemoryType(memRequirements.memoryTypeBits, buffer.Flags);

        if (vkAllocateMemory(RenderDevice::GetDevice(), &allocInfo, nullptr, &m_BufferUploader) != VK_SUCCESS) {
            return ErrorType::VULKAN_BUFFER_CREATION;
        }

        vkBindBufferMemory(RenderDevice::GetDevice(), m_Buffer, m_BufferUploader, 0);

        return ErrorType::SUCCESS;
    }

    void Buffer::Destroy()
    {
        vkDestroyBuffer(RenderDevice::GetDevice(), m_Buffer, nullptr);
        vkFreeMemory(RenderDevice::GetDevice(), m_BufferUploader, nullptr);
    }

    VkBuffer const & Buffer::GetBuffer() const
    {
        return m_Buffer;
    }

    VkDeviceMemory const & Buffer::GetBufferUploader() const
    {
        return m_BufferUploader;
    }

    void Buffer::Unmap()
    {
        vkUnmapMemory(RenderDevice::GetDevice(), m_BufferUploader);
    }

    ErrorType Buffer::Copy(Buffer const &buffer)
    {
        if (buffer.m_Size > m_Size)
            return ErrorType::VULKAN_BUFFER_CREATION;

        RenderCommands::BeginSingleTimeCommand();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = buffer.m_Size;
        vkCmdCopyBuffer(RenderCommands::GetCommandBuffer(), buffer.m_Buffer, m_Buffer, 1, &copyRegion);

        RenderCommands::EndSingleTimeCommand();
        return ErrorType::SUCCESS;
    }

} // GPC