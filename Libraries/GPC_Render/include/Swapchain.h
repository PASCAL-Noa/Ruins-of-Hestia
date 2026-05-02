#pragma once
#include <vulkan/vulkan.h>

#include "ErrorType.h"
#include "Object.h"
#include "Texture.h"

#include "Forward.h"

namespace GPC
{
	GPC_INHERIT_OBJECT(Swapchain)
	{

    public:
    	Swapchain() = default;
    	~Swapchain() override = default;

    	ErrorType Create(RenderWindow* window);
    	void Destroy();

		[[nodiscard]] VkExtent2D const& GetExtent2D() const { return m_SwapChainExtent; };
		[[nodiscard]] VkFormat const&	GetFormat() const { return m_SwapChainImageFormat; };

    	[[nodiscard]] VkSwapchainKHR const&	GetSwapchain() const { return m_Swapchain; }
    	[[nodiscard]] Texture*				GetTexture(uint32_t textureID) { return &m_SwapChainImages[textureID]; }
    	[[nodiscard]] size_t				GetImageCount() const { return m_SwapChainImages.size(); }

    private:

    	ErrorType CreateSwapChain();

    	// Swapchain datas
    	VkSwapchainKHR						m_Swapchain{};
    	std::vector<Texture>				m_SwapChainImages{};

    	VkFormat							m_SwapChainImageFormat;
    	VkExtent2D							m_SwapChainExtent{};

		RenderWindow*						m_RenderWindow;

    	VkSurfaceFormatKHR	ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    	VkPresentModeKHR	ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    	VkExtent2D			ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    };
}
