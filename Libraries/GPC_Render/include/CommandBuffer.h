#pragma once

#include "Object.h"
#include "ErrorType.h"

#include "vulkan/vulkan.h"
#include "Forward.h"

namespace GPC
{
    GPC_INHERIT_OBJECT(CommandBuffer)
    {

    public:
        CommandBuffer() = default;
        virtual ~CommandBuffer() override = default;

        ErrorType   Create(VkCommandPool m_linkedCommandPool);
        void        Destroy();

        VkCommandBuffer const& GetCommandBuffer();

    private:
        VkCommandPool       m_LinkedCommandPool;
        VkCommandBuffer     m_Buffer = { VK_NULL_HANDLE };

    };
}
