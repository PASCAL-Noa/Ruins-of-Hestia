#include "Texture.h"

#include <stb_image.h>

#include <stdexcept>

#include "Buffer.h"
#include "RenderCommands.h"
#include "RenderDevice.h"

namespace GPC
{
    Texture::Texture(){}

    Texture::~Texture(){}

    ErrorType Texture::CreateFromFile(std::string const &textureFile)
    {
        m_IsDestroyed = false;
        int texWidth, texHeight, texChannels;
        uint8_t* pixels = stbi_load(textureFile.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        m_Size = { texWidth, texHeight };

        if (!pixels) {
            return ErrorType::VULKAN_TEXTURE_LOAD;
        }

        Buffer buffer;
        buffer.Create("TEXTURE", {
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            imageSize
        });

        uint8_t* data;
        buffer.Map<uint8_t>(&data, 0, imageSize);
        memcpy(data, pixels, imageSize);
        buffer.Unmap();

        stbi_image_free(pixels);

        GPC_SUCCESS(CreateImage({
            static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),
            VK_SAMPLE_COUNT_1_BIT,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        }));


        GPC_SUCCESS(TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0));
        CopyBufferToImage(buffer.GetBuffer(), 0, 0, {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1});
        GPC_SUCCESS(TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0));

        buffer.Destroy();

        CreateImageView( {
            .Format     = VK_FORMAT_R8G8B8A8_SRGB,
            .ViewType   = VK_IMAGE_VIEW_TYPE_2D,
            .Flags      = VK_IMAGE_ASPECT_COLOR_BIT,
            .LayerCount = 1
        });

        return ErrorType::SUCCESS;
    }

    ErrorType Texture::CreateFromBlob(uint8_t *Data, uint64_t width, uint64_t height)
    {
        m_IsDestroyed = false;

        uint64_t imageSize = width * height * 4;
        m_Size = { width, height };

        Buffer buffer;
        buffer.Create("TEXTURE", {
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            imageSize
        });

        uint8_t* data;
        buffer.Map<uint8_t>(&data, 0, imageSize);
        memcpy(data, Data, imageSize);
        buffer.Unmap();

        GPC_SUCCESS(CreateImage({
            static_cast<uint32_t>(width), static_cast<uint32_t>(height),
            VK_SAMPLE_COUNT_1_BIT,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        }));


        GPC_SUCCESS(TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0));
        CopyBufferToImage(buffer.GetBuffer(), 0, 0, {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1});
        GPC_SUCCESS(TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0));

        buffer.Destroy();

        CreateImageView( {
            .Format     = VK_FORMAT_R8G8B8A8_SRGB,
            .ViewType   = VK_IMAGE_VIEW_TYPE_2D,
            .Flags      = VK_IMAGE_ASPECT_COLOR_BIT,
            .LayerCount = 1
        });

        return ErrorType::SUCCESS;
    }

    ErrorType Texture::Create(TextureCreationInformation const &information)
    {
        m_IsDestroyed = false;
        ErrorType error = ErrorType::SUCCESS;
        error = CreateImage(
            information.ImageInformation
        );
        GPC_SUCCESS(error);

        error = CreateImageView(information.ViewInformation);
        GPC_SUCCESS(error);

        RenderDevice::SetDebugName(VK_OBJECT_TYPE_IMAGE, information.Name, reinterpret_cast<uint64_t>(m_TextureImage));
        RenderDevice::SetDebugName(VK_OBJECT_TYPE_IMAGE_VIEW, information.Name + "_VIEW", reinterpret_cast<uint64_t>(m_TextureImageView));

        return error;
    }

    ErrorType Texture::Create(VkImage& baseImage, TextureViewInformation const &information)
    {
        m_IsDestroyed = false;
        m_TextureImage = baseImage;
        m_TextureFormat = information.Format;

        return CreateImageView(information);

    }

    void Texture::Destroy()
    {
        if (m_IsDestroyed) return;

        m_IsDestroyed = true;
        vkDestroyImageView(RenderDevice::GetDevice(), m_TextureImageView, nullptr);

        vkDestroyImage(RenderDevice::GetDevice(), m_TextureImage, nullptr);
        vkFreeMemory(RenderDevice::GetDevice(), m_TextureImageMemory, nullptr);
    }

    VkImageView const& Texture::GetImageView() const
    {
        return m_TextureImageView;
    }

    VkImage const & Texture::GetImage() const
    {
        return m_TextureImage;
    }

    uint32_t Texture::GetUploadId() const
    {
        return m_UploadId;
    }

    glm::u32vec2 Texture::GetSize() const
    {
        return m_Size;
    }

    void Texture::SetUploadId(uint32_t id)
    {
        m_UploadId = id;
    }

    ErrorType Texture::CreateImage(TextureImageInformation const& information) {

        m_TextureFormat = information.Format; // TODO CHANGE TO RGB IF JPEG

        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = information.Width;
        imageInfo.extent.height = information.Height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = information.LayerCount;
        imageInfo.format        = information.Format;
        imageInfo.tiling        = information.Tiling;
        imageInfo.initialLayout = information.Layout;
        imageInfo.usage         = information.Usage;
        imageInfo.samples       = information.NumSample;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags         = information.Flags;

        m_Size.x = information.Width;
        m_Size.y = information.Height;

        if (vkCreateImage(RenderDevice::GetDevice(), &imageInfo, nullptr, &m_TextureImage) != VK_SUCCESS) {
            return ErrorType::VULKAN_TEXTURE_CREATION;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(RenderDevice::GetDevice(), m_TextureImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = RenderDevice::FindMemoryType(memRequirements.memoryTypeBits, information.Properties);

        if (vkAllocateMemory(RenderDevice::GetDevice(), &allocInfo, nullptr, &m_TextureImageMemory) != VK_SUCCESS) {
            return ErrorType::VULKAN_BUFFER_ALLOCATION;
        }

        vkBindImageMemory(RenderDevice::GetDevice(), m_TextureImage, m_TextureImageMemory, 0);
        return ErrorType::SUCCESS;
    }

    ErrorType Texture::CreateImageView(TextureViewInformation const& view)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.viewType   = view.ViewType;
        viewInfo.image      = m_TextureImage;
        viewInfo.format     = view.Format;
        viewInfo.subresourceRange.aspectMask    = view.Flags;
        viewInfo.subresourceRange.baseMipLevel  = 0;
        viewInfo.subresourceRange.levelCount    = 1;
        viewInfo.subresourceRange.baseArrayLayer= 0;
        viewInfo.subresourceRange.layerCount    = view.LayerCount;

        if (vkCreateImageView(RenderDevice::GetDevice(), &viewInfo, nullptr, &m_TextureImageView) != VK_SUCCESS) {
            return ErrorType::VULKAN_TEXTURE_VIEW_CREATION;
        }
        return ErrorType::SUCCESS;
    }

    ErrorType Texture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layout) {

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_TextureImage;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = layout;
        barrier.subresourceRange.layerCount     = 1;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (RenderDevice::HasStencilComponent(m_TextureFormat)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            return ErrorType::VULKAN_TEXTURE_TRANSITION;
        }

        RenderCommands::BeginSingleTimeCommand();
        vkCmdPipelineBarrier(
            RenderCommands::GetCommandBuffer(),
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        RenderCommands::EndSingleTimeCommand();

        return ErrorType::SUCCESS;
    }

    void Texture::CopyBufferToImage(VkBuffer buffer, uint32_t offset, uint32_t layerOffset, VkExtent3D dimensions) {

        VkBufferImageCopy region{};
        region.bufferOffset         = offset;
        region.bufferRowLength      = 0;
        region.bufferImageHeight    = 0;
        region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel        = 0;
        region.imageSubresource.baseArrayLayer  = layerOffset;
        region.imageSubresource.layerCount      = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = dimensions;

        RenderCommands::BeginSingleTimeCommand();

        vkCmdCopyBufferToImage(
            RenderCommands::GetCommandBuffer(),
            buffer, m_TextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region
            );

        RenderCommands::EndSingleTimeCommand();

    }
} // GPC