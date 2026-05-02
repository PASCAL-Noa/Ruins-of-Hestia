#include "../include/RenderDevice.h"

#include <cstring>
#include <map>
#include <set>
#include <GLFW/glfw3.h>

#include "Window.h"

namespace GPC
{

    ErrorType RenderDevice::Create(Window* linkedWindow)
    {

        if (m_VulkanInstance != VK_NULL_HANDLE) return ErrorType::VULKAN_ALREADY_ALLOCATED;

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = "GPC_Render_Ruins_Of_Hestia";
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName = "GPC_Render_Engine";
        applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo vulkanInstanceInfo{};
        vulkanInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        vulkanInstanceInfo.pApplicationInfo = &applicationInfo;
        if (UseValidationLayer) {
            vulkanInstanceInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
            vulkanInstanceInfo.ppEnabledLayerNames = ValidationLayers.data();
        }
        else {
            vulkanInstanceInfo.enabledLayerCount = 0;
        }

        FindRequirements();

        // Debug
        std::cout << "Extensions count: " << m_GlfwRequirements.size() << std::endl;
        for (const auto& ext : m_GlfwRequirements) {
            std::cout << "Extension: " << ext << std::endl;
        }

        // Vérifie que ces extensions sont bien disponibles
        uint32_t availableCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &availableCount, nullptr);
        std::vector<VkExtensionProperties> available(availableCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &availableCount, available.data());
        std::cout << "Available extensions: " << availableCount << std::endl;
        for (const auto& ext : available) {
            std::cout << "  " << ext.extensionName << std::endl;
        }
        vulkanInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(m_GlfwRequirements.size());
        vulkanInstanceInfo.ppEnabledExtensionNames = m_GlfwRequirements.data();

        // Create Vulkan instance
        if (VkResult result = vkCreateInstance(&vulkanInstanceInfo, nullptr, &m_VulkanInstance); result != VK_SUCCESS)
            return ErrorType::VULKAN_DEVICE_INSTANCE_CREATION;

        // Check for validations layers (Used for debug because Vulkan don't support them)
        if (UseValidationLayer && !CheckValidationSupport())
            return ErrorType::VULKAN_DEVICE_VALIDATION_LAYER;

        GPC_SUCCESS(CreateDebugLayer());

        m_LinkedSurface = new VkSurfaceKHR();
        ErrorType err = linkedWindow->GetSurface(&m_LinkedSurface);
        if (GPC_FAILED(err)) return err;

        GPC_SUCCESS(CreatePhysicalDevice());
        GPC_SUCCESS(CreateLogicalDevice());

        return ErrorType::SUCCESS;

    }

    void RenderDevice::Destroy()
    {

        vkDestroyDevice(m_Device, nullptr);

        if (UseValidationLayer) {
            DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, nullptr);
        }

        vkDestroyInstance(m_VulkanInstance, nullptr);

    }

    size_t RenderDevice::GetDynamicAllocationSize(size_t size)
    {
        size_t minUboAlignment = GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
        size_t m_dynamicBufferAlignment = size;
        if (minUboAlignment > 0) {
            m_dynamicBufferAlignment = (m_dynamicBufferAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
        }
        return m_dynamicBufferAlignment;
    }

    ErrorType RenderDevice::AlignedDataAllocation(void **data, size_t size, size_t alignment)
    {
#if defined(_MSC_VER) || defined(__MINGW32__)
        *data = _aligned_malloc(size, alignment);
#else
        int res = posix_memalign(data, alignment, size);
        if (res != 0) {
            data = nullptr;
            return ErrorType::VULKAN_BUFFER_ALLOCATION;
        }
#endif
        return ErrorType::SUCCESS;
    }

    bool RenderDevice::HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    VkSampleCountFlagBits const & RenderDevice::GetSampleCountFlag()
    {
        FindMaxUsableSample();

        return m_MsaaSamples;
    }

    VkFormat const & RenderDevice::GetDepthStencilFormat()
    {
        if (!m_FoundDepthStencil) FindDepthStencilFormat();
        m_FoundDepthStencil = true;

        return m_DepthStencilFormat;
    }

    void RenderDevice::WaitIdle()
    {
        if (vkDeviceWaitIdle(m_Device) != VK_SUCCESS) { // TODO VkResult séparé  pour faire une macro ASSERT_DEBUG()
            GPC_ERROR << "Wait of device fail!" << ENDL;
        }
    }

    VkFormat RenderDevice::FindImageSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                                    VkFormatFeatureFlags features)
    {

        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(GetPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!"); // TODO RETURN ERROR TAKE PARAMETER OF POINTER
    }

    uint32_t RenderDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(GetPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!"); // TODO RETURN ERROR TAKE PARAMETER OF POINTER
    }

    void RenderDevice::FindDepthStencilFormat()
    {
       m_DepthStencilFormat = FindImageSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    ErrorType RenderDevice::CreatePhysicalDevice()
    {
        if (m_PhysicalDevice) return ErrorType::SUCCESS;

        // Get all available GPU on computer
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, devices.data());
        if (deviceCount == 0) {
            return ErrorType::VULKAN_DEVICE_NO_SUITABLE_GPU;
        }

        std::multimap<int, VkPhysicalDevice> candidates;

        for (const auto& device : devices) {
            int score = RatePhysicalDeviceSuitability(device);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0) {
            m_PhysicalDevice = candidates.rbegin()->second;
            FindMaxUsableSample();
        } else {
            return ErrorType::VULKAN_DEVICE_NO_SUITABLE_GPU;
        }

        return ErrorType::SUCCESS;
    }

    ErrorType RenderDevice::CreateLogicalDevice()
    {
        if (m_Device) return ErrorType::SUCCESS;

        FindQueueFamilies(m_PhysicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set uniqueQueueFamilies = {
                m_QueueFamilyIndices.graphicsFamily.value(),
                m_QueueFamilyIndices.presentFamily.value()
            };

        if (!m_QueueFamilyIndices.HasValues())
            return ErrorType::VULKAN_DEVICE_CREATION_FAILED;

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.fillModeNonSolid = VK_TRUE;
        deviceFeatures.geometryShader = VK_TRUE;
        deviceFeatures.imageCubeArray = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

        if (UseValidationLayer) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
            createInfo.ppEnabledLayerNames = ValidationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
            return ErrorType::VULKAN_DEVICE_CREATION_FAILED;
        }

        vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.presentFamily.value(), 0, &m_PresentQueue);
        vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.computeFamily.value(), 0, &m_ComputeQueue);

        return ErrorType::SUCCESS;

    }

    void RenderDevice::FindQueueFamilies(VkPhysicalDevice device)
    {

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                m_QueueFamilyIndices.graphicsFamily = i;
            }
            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                m_QueueFamilyIndices.computeFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *m_LinkedSurface, &presentSupport);

            if (presentSupport)
            {
                m_QueueFamilyIndices.presentFamily = i;
            }
            if (m_QueueFamilyIndices.HasValues()) break;
            i++;
        }

    }

    void RenderDevice::FindSwapChainSupport(VkPhysicalDevice device)
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, *m_LinkedSurface, &m_SwapChainSupportDetails.Capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, *m_LinkedSurface, &formatCount, nullptr);

        if (formatCount != 0) {
            m_SwapChainSupportDetails.Formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, *m_LinkedSurface, &formatCount, m_SwapChainSupportDetails.Formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, *m_LinkedSurface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            m_SwapChainSupportDetails.PresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, *m_LinkedSurface, &presentModeCount, m_SwapChainSupportDetails.PresentModes.data());
        }
    }

    void RenderDevice::SetDebugName(VkObjectType type, std::string_view name, uint64_t objectHandle)
    {

#ifndef NDEBUG
        if (vkSetDebugUtilsObjectNameEX == nullptr) {
            vkSetDebugUtilsObjectNameEX = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>( vkGetInstanceProcAddr( RenderDevice::GetVulkanInstance(), "vkSetDebugUtilsObjectNameEXT" ) );
        }
        VkDebugUtilsObjectNameInfoEXT debugInfo {};
        debugInfo.sType         = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugInfo.pNext         = nullptr;
        debugInfo.objectType    = type;
        debugInfo.objectHandle  = objectHandle;
        debugInfo.pObjectName   = name.data();
        vkSetDebugUtilsObjectNameEX(GetDevice(), &debugInfo);
#endif
    }

    int RenderDevice::RatePhysicalDeviceSuitability(VkPhysicalDevice device)
    {
        vkGetPhysicalDeviceProperties(device, &m_DeviceProperties);
        vkGetPhysicalDeviceFeatures(device, &m_DeviceFeatures);

        bool extensionsSupported = CheckDeviceExtensionSupport(device);
        FindQueueFamilies(device);

        int score = 0;

        // Discrete GPUs have a significant performance advantage
        if (m_DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            FindSwapChainSupport(device);
            swapChainAdequate = !m_SwapChainSupportDetails.Formats.empty() && !m_SwapChainSupportDetails.PresentModes.empty();
        }

        // Maximum possible size of textures affects graphics quality
        score += m_DeviceProperties.limits.maxImageDimension2D;

        // Application can't function without geometry shaders
        if (!m_DeviceFeatures.geometryShader
            || !m_QueueFamilyIndices.HasValues()
            || !extensionsSupported
            || !swapChainAdequate) {
            return 0;
            }

        return score;
    }

    bool RenderDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool RenderDevice::CheckValidationSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : ValidationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    void RenderDevice::FindRequirements()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        m_GlfwRequirements = std::vector(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (UseValidationLayer) {
            m_GlfwRequirements.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
    }

    void RenderDevice::FindMaxUsableSample()
    {
        if (m_FoundMsaaSamples) return;
        m_FoundMsaaSamples = true;

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { m_MsaaSamples = VK_SAMPLE_COUNT_64_BIT; }
        else if (counts & VK_SAMPLE_COUNT_32_BIT) { m_MsaaSamples = VK_SAMPLE_COUNT_32_BIT; }
        else if (counts & VK_SAMPLE_COUNT_16_BIT) { m_MsaaSamples = VK_SAMPLE_COUNT_16_BIT; }
        else if (counts & VK_SAMPLE_COUNT_8_BIT) { m_MsaaSamples = VK_SAMPLE_COUNT_8_BIT; }
        else if (counts & VK_SAMPLE_COUNT_4_BIT) { m_MsaaSamples = VK_SAMPLE_COUNT_4_BIT; }
        else if (counts & VK_SAMPLE_COUNT_2_BIT) { m_MsaaSamples = VK_SAMPLE_COUNT_2_BIT; }
        else m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    ErrorType RenderDevice::CreateDebugLayer()
    {
        if (!UseValidationLayer) return ErrorType::SUCCESS;

        // Debug layer
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugInfo.pfnUserCallback = DebugMessageCallback;
        // debugInfo.pUserData = this; // TODO POTENTIAL ADDITIONAL USER DATA

        if (CreateDebugUtilsMessengerEXT(m_VulkanInstance, &debugInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
            return ErrorType::VULKAN_DEVICE_DEBUG_CREATION;
        }

        return ErrorType::SUCCESS;
    }

    VkBool32 RenderDevice::DebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                         VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                         void *pUserData)
    {

        // VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance
        // VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake
        // VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan

        // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
        // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the creation of a resource
        // VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior that is not necessarily an error, but very likely a bug in your application
        // VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes

        VkDebugUtilsMessengerCallbackDataEXT const* data = pCallbackData;
        switch (messageSeverity) {

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                GPC_ERROR << "[Vulkan] At \n" << data->pMessage << ENDL;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                GPC_WARNING << "[Vulkan] At \n" << data->pMessage << ENDL;
                break;
                default:break;

        }

        return VK_FALSE;

    }

    VkResult RenderDevice::CreateDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT*pDebugMessenger)
    {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void RenderDevice::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator)
    {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
} // GPC