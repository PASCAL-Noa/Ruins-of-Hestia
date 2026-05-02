#include "../include/TextureArray.h"
#include "Buffer.h"
#include "../../GPC_Assets/include/Assets.h"

namespace GPC
{
    ImageInformation::ImageInformation(const TextureAsset* asset)
    {
        if (asset) {
            Width  = asset->Width;
            Height = asset->Height;
            Stride = asset->Channels;
            Data   = asset->Data.Ptr();
        } else {
            Width = 0;
            Height = 0;
            Stride = 0;

            Data = nullptr;
        }
    }
    uint32_t TextureArray::AddImage(ImageInformation const & info, uint32_t layer)
    {

        if (m_Layers.size() <= layer) m_Layers.resize(layer + 1);

        uint32_t offset = m_TotalSize;
        m_TotalSize += info.Width * info.Height * info.Stride;
        m_Layers[layer] = info;
        return layer;

    }

    ErrorType TextureArray::Create(TextureArrayInformation const &textureArray)
    {
        const uint32_t layerCount = m_Layers.size();

        GPC::TextureCreationInformation information {
            .Name = std::to_string(textureArray.Width) + "x" + std::to_string(textureArray.Height),
            .ViewInformation = {
                .Format = textureArray.Format,
                .ViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                .Flags = VK_IMAGE_ASPECT_COLOR_BIT,
                .LayerCount = layerCount,
            },
            .ImageInformation = {
                .Width = textureArray.Width,
                .Height = textureArray.Height,
                .NumSample = VK_SAMPLE_COUNT_1_BIT,
                .Format = textureArray.Format,
                .Tiling = VK_IMAGE_TILING_OPTIMAL,
                .Usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                .LayerCount = layerCount
            },
        };

        CreateImage(information.ImageInformation);

        Buffer buffer;
        buffer.Create("TEXTURE_ARRAY_UPLOADER", {
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_TotalSize
        });

        uint64_t currentOffset = 0;

        uint8_t* data;
        buffer.Map<uint8_t>(&data, 0, m_TotalSize);

        for (int layerIndex = 0; layerIndex < layerCount; layerIndex++) {

            ImageInformation const& info = m_Layers[layerIndex];
            uint64_t imageSize = info.Width * info.Height * info.Stride;

            if (imageSize == 0)
                continue;
            memcpy(data + currentOffset, info.Data, imageSize);


            GPC_SUCCESS(TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerIndex));
            CopyBufferToImage(buffer.GetBuffer(), currentOffset, layerIndex, {static_cast<uint32_t>(info.Width), static_cast<uint32_t>(info.Height), 1});
            GPC_SUCCESS(TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layerIndex));

            currentOffset += imageSize;

        }
        buffer.Unmap();

        CreateImageView(information.ViewInformation); // TODO FAIRE LES SUCCESS

        buffer.Destroy();

        return ErrorType::SUCCESS;

    }

    void TextureArray::Destroy()
    {
        Texture::Destroy();
    }
} // GPC