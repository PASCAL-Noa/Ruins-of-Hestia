#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

#include "ErrorType.h"
#include "Object.h"

#include "Forward.h"

namespace GPC
{

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily  ;
        std::optional<uint32_t> computeFamily  ;
        std::optional<uint32_t> presentFamily   ;

        [[nodiscard]] bool HasValues() const
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;
    };

    GPC_INHERIT_OBJECT(RenderDevice)
    {

    public:
        ~RenderDevice() override = default;

        static ErrorType   Create(Window* linkedWindow);
        static void        Destroy();

        static VkInstance       const& GetVulkanInstance()  { return m_VulkanInstance; };

        static VkDevice         const& GetDevice()          { return m_Device; };
        static VkPhysicalDevice const& GetPhysicalDevice() { return m_PhysicalDevice; };

        static VkQueue          const& GetGraphicQueue() { return m_GraphicsQueue; };
        static VkQueue          const& GetPresentQueue() { return m_PresentQueue; };
        static VkQueue          const& GetComputeQueue() { return m_ComputeQueue; };

        static std::vector<const char*> const& GetDeviceExtensions() { return m_GlfwRequirements; }

        static VkPhysicalDeviceProperties   const&  GetPhysicalDeviceProperties(){ return m_DeviceProperties; };
        static VkPhysicalDeviceFeatures     const&  GetPhysicalDeviceFeatures()  { return m_DeviceFeatures; };
        static VkSampleCountFlagBits        const&  GetSampleCountFlag();
        static VkFormat                     const&  GetDepthStencilFormat();

        static QueueFamilyIndices           const&  GetQueueFamilyIndices()      { return m_QueueFamilyIndices; };
        static SwapChainSupportDetails      const&  GetSwapChainSupportDetails() { return m_SwapChainSupportDetails; };

        static void                                 WaitIdle();

        static VkFormat     FindImageSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        static uint32_t     FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        static void         FindDepthStencilFormat();

        static size_t       GetDynamicAllocationSize(size_t size);
        static ErrorType    AlignedDataAllocation(void **data, size_t size, size_t alignment);
        static bool         HasStencilComponent(VkFormat format);

        static void FindSwapChainSupport(VkPhysicalDevice device);
        static void SetDebugName(VkObjectType type, std::string_view name, uint64_t objectHandle);

    private:
        RenderDevice() = default;

        ///////// DEVICE CREATION /////////
        static ErrorType CreatePhysicalDevice();
        static ErrorType CreateLogicalDevice();

        static void FindQueueFamilies(VkPhysicalDevice device);

        static int  RatePhysicalDeviceSuitability(VkPhysicalDevice device);
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        static bool CheckValidationSupport();

        static void FindRequirements();
        static void FindMaxUsableSample() ;

        ///////// DEBUG /////////
        static ErrorType CreateDebugLayer();

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
        static VkResult    CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        static void        DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    private:
        static inline VkInstance                        m_VulkanInstance    = VK_NULL_HANDLE;

        static inline VkPhysicalDevice                  m_PhysicalDevice    = VK_NULL_HANDLE;
        static inline VkDevice                          m_Device            = VK_NULL_HANDLE;

        static inline VkQueue                           m_PresentQueue      = VK_NULL_HANDLE;
        static inline VkQueue                           m_GraphicsQueue     = VK_NULL_HANDLE;
        static inline VkQueue                           m_ComputeQueue      = VK_NULL_HANDLE;
        static inline VkSurfaceKHR*                     m_LinkedSurface     = nullptr;

        static inline VkSampleCountFlagBits             m_MsaaSamples       = VK_SAMPLE_COUNT_1_BIT;
        static inline VkFormat                          m_DepthStencilFormat= VK_FORMAT_D32_SFLOAT;
        static inline bool                              m_FoundDepthStencil = false;
        static inline bool                              m_FoundMsaaSamples  = false;

        static inline VkPhysicalDeviceProperties        m_DeviceProperties  {};
        static inline VkPhysicalDeviceFeatures          m_DeviceFeatures    {};

        static inline VkDebugUtilsMessengerEXT          m_DebugMessenger    = VK_NULL_HANDLE;
        static inline std::vector<const char*>          m_GlfwRequirements  = {};

        static inline QueueFamilyIndices                m_QueueFamilyIndices = {};
        static inline SwapChainSupportDetails           m_SwapChainSupportDetails = {};

    public:
#ifdef NDEBUG
        static constexpr bool UseValidationLayer = false;
#else
        static constexpr bool UseValidationLayer = true;
        static inline PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEX = nullptr;
#endif

        static inline std::vector<const char*> DeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        static inline std::vector<const char*> ValidationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
    };
} // GPC