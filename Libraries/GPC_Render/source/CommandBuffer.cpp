#include "CommandBuffer.h"

#include "Buffer.h"

namespace GPC
{

    ErrorType CommandBuffer::Create(VkCommandPool linkedCommandPool)
    {

        m_LinkedCommandPool = linkedCommandPool;

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_LinkedCommandPool;
        allocInfo.commandBufferCount = 1;

        VkResult result = vkAllocateCommandBuffers(RenderDevice::GetDevice(), &allocInfo, &m_Buffer);

        RenderDevice::SetDebugName(VK_OBJECT_TYPE_COMMAND_BUFFER, "COMMAND_BUFFER", reinterpret_cast<uint64_t>(m_Buffer));

        if (result != VK_SUCCESS)
            return ErrorType::VULKAN_CONTEXT_COMMAND_POOL;
        return ErrorType::SUCCESS;
    }

    void CommandBuffer::Destroy()
    {

        vkFreeCommandBuffers(RenderDevice::GetDevice(), m_LinkedCommandPool, 1, &m_Buffer);

    }

    VkCommandBuffer const & CommandBuffer::GetCommandBuffer()
    {
        return m_Buffer;
    }
}
