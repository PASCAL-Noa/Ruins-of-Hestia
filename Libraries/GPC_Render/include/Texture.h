#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "ErrorType.h"
#include "Object.h"

namespace GPC
{

    struct TextureViewInformation
    {
        VkFormat                Format      = VK_FORMAT_UNDEFINED;
        VkImageViewType         ViewType    = VK_IMAGE_VIEW_TYPE_2D;
        VkFlags   Flags                     = VK_IMAGE_ASPECT_NONE;

        uint32_t                LayerCount  = 1;
    };

    struct TextureImageInformation
    {
        uint32_t                Width       = 512;
        uint32_t                Height      = 512;
        VkSampleCountFlagBits   NumSample   = VK_SAMPLE_COUNT_1_BIT;
        VkFormat                Format      = VK_FORMAT_UNDEFINED;
        VkImageTiling           Tiling      = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags       Usage       = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        VkMemoryPropertyFlags   Properties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VkImageLayout           Layout      = VK_IMAGE_LAYOUT_UNDEFINED;

        uint32_t                LayerCount  = 1;
        VkImageCreateFlagBits   Flags       = {};
    };

    enum SampleCount
    {
        COUNT_1_BIT  = VK_SAMPLE_COUNT_1_BIT ,
        COUNT_2_BIT  = VK_SAMPLE_COUNT_2_BIT ,
        COUNT_4_BIT  = VK_SAMPLE_COUNT_4_BIT ,
        COUNT_8_BIT  = VK_SAMPLE_COUNT_8_BIT ,
        COUNT_16_BIT = VK_SAMPLE_COUNT_16_BIT,
        COUNT_32_BIT = VK_SAMPLE_COUNT_32_BIT,
        COUNT_64_BIT = VK_SAMPLE_COUNT_64_BIT
    };

    struct TextureCreationInformation
    {
        std::string Name;
        TextureViewInformation  ViewInformation = {};
        TextureImageInformation ImageInformation= {};
    };

    GPC_INHERIT_OBJECT(Texture)
    {
    public:
        Texture();
        ~Texture() override;

        virtual ErrorType CreateFromFile(std::string const& textureFile);
        virtual ErrorType CreateFromBlob(uint8_t *Data, uint64_t width, uint64_t height);
        virtual ErrorType Create(TextureCreationInformation const& information);
        virtual ErrorType Create(VkImage& baseImage, TextureViewInformation const& information);
        virtual void Destroy();

        ErrorType TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layout);

        [[nodiscard]] VkImageView const& GetImageView() const;
        [[nodiscard]] VkImage const& GetImage() const;

        uint32_t GetUploadId() const;
        glm::u32vec2 GetSize() const;
        void SetUploadId(uint32_t id);

    protected:
        VkImage         m_TextureImage          = { VK_NULL_HANDLE };
        VkImageView     m_TextureImageView      = { VK_NULL_HANDLE };
        VkDeviceMemory  m_TextureImageMemory    = { VK_NULL_HANDLE };

        VkFormat        m_TextureFormat         = VK_FORMAT_UNDEFINED;
        bool            m_IsDestroyed           = false;

        uint32_t        m_UploadId              = 0;
        glm::u32vec2    m_Size                  = { 0, 0 };

        ErrorType CreateImage(TextureImageInformation const& information);
        ErrorType CreateImageView(TextureViewInformation const& view);

        void CopyBufferToImage(VkBuffer buffer, uint32_t offset, uint32_t layerOffset, VkExtent3D dimensions);
    };

} // GPC