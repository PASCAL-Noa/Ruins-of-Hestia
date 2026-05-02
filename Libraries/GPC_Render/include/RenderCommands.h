#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "CommandBuffer.h"
#include "ErrorType.h"
#include "Object.h"

#include "Forward.h"

namespace GPC
{

    GPC_INHERIT_OBJECT(RenderCommands)
    {

    public:
        RenderCommands() = default;
        ~RenderCommands() override;


        static ErrorType        Create();
        static void             Destroy();

        static void             BeginSingleTimeCommand();
        static void             EndSingleTimeCommand();

        static void             BeginCommands();
        static void             EndCommands();

        static void             BeginComputeCommands();
        static void             EndComputeCommands();

        static VkCommandBuffer  GetCommandBuffer();
        static VkCommandBuffer  GetComputeCommandBuffer();

        static uint32_t         GetCurrentFrame();

    private:
        static ErrorType        CreateCommandPool();

        static ErrorType        CreateCommandBuffer();
        static ErrorType        CreateComputeCommandBuffer();

        static void             SetCurrentFrame(uint32_t* frame);

        // Command list
        static inline std::vector<CommandBuffer> m_CommandBuffers = {};
        static inline std::vector<CommandBuffer> m_ComputeCommandBuffers = {};
        static inline VkCommandPool m_CommandPool   = { VK_NULL_HANDLE };

        static inline uint32_t* mp_CurrentFrame     = { nullptr };

        friend RenderWindow;

    };

} // GPC