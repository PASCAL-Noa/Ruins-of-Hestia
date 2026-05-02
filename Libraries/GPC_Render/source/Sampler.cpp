#include "../include/Sampler.h"

#include "RenderDevice.h"

namespace GPC
{
    ErrorType Sampler::Create(const SamplerInformation& information)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(RenderDevice::GetPhysicalDevice(), &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = information.Type;
        samplerInfo.magFilter               = information.MagFilter;
        samplerInfo.minFilter               = information.MinFilter;
        samplerInfo.addressModeU            = information.AddressModeU;
        samplerInfo.addressModeV            = information.AddressModeV;
        samplerInfo.addressModeW            = information.AddressModeW;
        samplerInfo.anisotropyEnable        = information.AnisotropyEnable;
        samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor             = information.BorderColor;
        samplerInfo.unnormalizedCoordinates = information.UnnormalizedCoordinates;
        samplerInfo.compareEnable           = information.CompareEnable;
        samplerInfo.compareOp               = information.CompareOp;
        samplerInfo.mipmapMode              = information.MipmapMode;
        samplerInfo.mipLodBias              = information.MipLodBias;
        samplerInfo.minLod                  = information.MinLod;
        samplerInfo.maxLod                  = information.MaxLod;

        if (vkCreateSampler(RenderDevice::GetDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) {
            return ErrorType::VULKAN_SAMPLER_CREATION;
        }
        return ErrorType::SUCCESS;
    }

    void Sampler::Destroy()
    {
        vkDestroySampler(RenderDevice::GetDevice(), m_TextureSampler, nullptr);
    }
} // GPC