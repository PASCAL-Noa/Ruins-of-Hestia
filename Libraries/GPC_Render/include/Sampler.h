#pragma once
#include <vulkan/vulkan.h>

#include "ErrorType.h"
#include "Object.h"

struct SamplerInformation
{
    VkStructureType         Type                    = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    VkFilter                MagFilter               = VK_FILTER_LINEAR;
    VkFilter                MinFilter               = VK_FILTER_LINEAR;
    VkSamplerAddressMode    AddressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode    AddressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode    AddressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkBool32                AnisotropyEnable        = VK_TRUE;
    VkBorderColor           BorderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    VkBool32                UnnormalizedCoordinates = VK_FALSE;
    VkBool32                CompareEnable           = VK_FALSE;
    VkCompareOp             CompareOp               = VK_COMPARE_OP_ALWAYS;
    VkSamplerMipmapMode     MipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    float                   MipLodBias              = 0.0f;
    float                   MinLod                  = 0.0f;
    float                   MaxLod                  = 0.0f;
};

namespace GPC
{
    GPC_INHERIT_OBJECT( Sampler )
    {
    public:
        Sampler() = default;
        ~Sampler() override = default;

        ErrorType   Create(const SamplerInformation& information);
        void        Destroy();

        VkSampler const&  GetSampler() const { return m_TextureSampler; };

    private:
        VkSampler m_TextureSampler = { VK_NULL_HANDLE };
    };

} // GPC