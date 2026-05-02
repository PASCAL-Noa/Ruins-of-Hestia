#include "RenderCommands.h"

#include "Render.h"
#include "RenderDevice.h"
#include "CommandBuffer.h"

#include "GPC_Framework.h"

namespace GPC
{
    RenderCommands::~RenderCommands()
    {
    }

    ErrorType RenderCommands::Create()
    {

        GPC_SUCCESS(CreateCommandPool());

        GPC_SUCCESS(CreateCommandBuffer());

        GPC_SUCCESS(CreateComputeCommandBuffer());

        return ErrorType::SUCCESS;

    }

    void RenderCommands::Destroy()
    {
        for (auto& commandBuffer : m_CommandBuffers) {
            commandBuffer.Destroy();
        }

        vkDestroyCommandPool(RenderDevice::GetDevice(), m_CommandPool, nullptr);
    }

    void RenderCommands::BeginSingleTimeCommand()
    {

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(GetCommandBuffer(), &beginInfo);

    }

    void RenderCommands::EndSingleTimeCommand()
    {
        VkCommandBuffer buffer = GetCommandBuffer();
        vkEndCommandBuffer(buffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &buffer;

        vkQueueSubmit(RenderDevice::GetGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(RenderDevice::GetGraphicQueue());

        vkResetCommandBuffer(buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

    }

    void RenderCommands::BeginCommands()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        // Flags can be
        // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
        // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.

        if (vkBeginCommandBuffer(GetCommandBuffer(), &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    }

    void RenderCommands::EndCommands()
    {

        if (vkEndCommandBuffer(GetCommandBuffer()) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

    }

    void RenderCommands::BeginComputeCommands()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        // Flags can be
        // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
        // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.

        if (vkBeginCommandBuffer(GetComputeCommandBuffer(), &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    }

    void RenderCommands::EndComputeCommands()
    {
        if (vkEndCommandBuffer(GetComputeCommandBuffer()) != VK_SUCCESS) {
            throw std::runtime_error("failed to record compute command buffer!");
        }
    }

    VkCommandBuffer RenderCommands::GetCommandBuffer()
    {
        return m_CommandBuffers[*mp_CurrentFrame].GetCommandBuffer();
    }

    VkCommandBuffer RenderCommands::GetComputeCommandBuffer()
    {
        return m_ComputeCommandBuffers[*mp_CurrentFrame].GetCommandBuffer();
    }

    uint32_t RenderCommands::GetCurrentFrame()
    {
        return *mp_CurrentFrame;
    }

    ErrorType RenderCommands::CreateCommandBuffer()
    {

        m_CommandBuffers.resize(MAX_GENERATED_FRAME);
        for (auto& commandBuffer : m_CommandBuffers) {
            GPC_SUCCESS(commandBuffer.Create(m_CommandPool));
        }

        return ErrorType::SUCCESS;

    }

    ErrorType RenderCommands::CreateComputeCommandBuffer()
    {
        m_ComputeCommandBuffers.resize(MAX_GENERATED_FRAME);
        for (auto& commandBuffer : m_ComputeCommandBuffers) {
            GPC_SUCCESS(commandBuffer.Create(m_CommandPool));
        }
        return ErrorType::SUCCESS;
    }

    ErrorType RenderCommands::CreateCommandPool()
    {

        QueueFamilyIndices const& queueFamilyIndices = RenderDevice::GetQueueFamilyIndices();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(RenderDevice::GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
            return ErrorType::VULKAN_CONTEXT_COMMAND_POOL;
        }

        RenderDevice::SetDebugName(VK_OBJECT_TYPE_COMMAND_POOL, "COMMAND_POOL", reinterpret_cast<uint64_t>(m_CommandPool));

        return ErrorType::SUCCESS;

    }


    void RenderCommands::SetCurrentFrame(uint32_t *frame)
    {
        mp_CurrentFrame = frame;
    }
} // GPC