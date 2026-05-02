#include "Swapchain.h"

#include <GLFW/glfw3.h>

#include "Defines.h"
#include "RenderDevice.h"
#include "RenderWindow.h"

namespace GPC
{
    ErrorType Swapchain::Create(RenderWindow* window)
    {

        m_RenderWindow = window;
        RenderDevice::FindSwapChainSupport(RenderDevice::GetPhysicalDevice());

        return CreateSwapChain();

    }

    void Swapchain::Destroy()
    {

        for (auto& swapChainImage : m_SwapChainImages)
            vkDestroyImageView(RenderDevice::GetDevice(),   swapChainImage.GetImageView(), nullptr);

        vkDestroySwapchainKHR(RenderDevice::GetDevice(),    m_Swapchain, nullptr);

    }

    ErrorType Swapchain::CreateSwapChain()
    {

        SwapChainSupportDetails const& swapChainSupport = RenderDevice::GetSwapChainSupportDetails();

        VkSurfaceFormatKHR surfaceFormat    = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
        VkPresentModeKHR presentMode        = ChooseSwapPresentMode(swapChainSupport.PresentModes);
        VkExtent2D extent                   = ChooseSwapExtent(swapChainSupport.Capabilities);

        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;

        uint32_t imageCount = MAX_GENERATED_FRAME;
        if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount) {
            imageCount = swapChainSupport.Capabilities.maxImageCount;
        }

        VkSurfaceKHR* surface;
        GPC_SUCCESS(m_RenderWindow->GetSurface(&surface));

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = *surface;

        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = surfaceFormat.format;
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;
        createInfo.imageExtent      = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = RenderDevice::GetQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE; // TODO OLD SWAPCHAIN

        VkResult result = vkCreateSwapchainKHR(RenderDevice::GetDevice(), &createInfo, nullptr, &m_Swapchain);
        if (result != VK_SUCCESS) {
            return ErrorType::VULKAN_RENDER_SWAP_CHAIN_CREATION;
        }

        vkGetSwapchainImagesKHR(RenderDevice::GetDevice(), m_Swapchain, &imageCount, nullptr);
        std::vector<VkImage> swapchain(imageCount);
        vkGetSwapchainImagesKHR(RenderDevice::GetDevice(), m_Swapchain, &imageCount, swapchain.data());

        m_SwapChainImages.resize(imageCount);
        for (int i = 0; i < imageCount; i++) {

            TextureViewInformation viewInformation;
            viewInformation.Format = m_SwapChainImageFormat;
            viewInformation.Flags = VK_IMAGE_ASPECT_COLOR_BIT;
            m_SwapChainImages[i].Create(swapchain[i], viewInformation);
        }

        return ErrorType::SUCCESS;

    }

    VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            // We ue mailing method because remove tearing and render fastest as possible
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(m_RenderWindow->ToGLFWWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
}
