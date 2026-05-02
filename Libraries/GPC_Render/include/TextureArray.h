#pragma once
#include "Object.h"
#include "Texture.h"

namespace GPC
{
    struct TextureAsset;

    using TextureID = uint32_t;

    struct ImageInformation
    {
        uint8_t*    Data;

        uint32_t    Width;
        uint32_t    Height;
        uint32_t    Stride;

        ImageInformation() = default;
        ImageInformation(const TextureAsset* asset);
    };

    struct TextureArrayInformation
    {
        uint32_t    Width   = 0;
        uint32_t    Height  = 0;
        VkFormat    Format  = VK_FORMAT_R8G8B8A8_SRGB;
    };

    GPC_INHERIT_OBJECT(TextureArray), public Texture
    {
    public:
        TextureArray() = default;
        ~TextureArray() override = default;


        uint32_t AddImage(ImageInformation const& info, uint32_t layer);
        ErrorType Create(TextureArrayInformation const& textureArray);

        void Destroy() override;

    private:

        uint32_t                        m_TotalSize = 0;
        std::vector<ImageInformation>   m_Layers;

    private:
        ErrorType Create(TextureCreationInformation const &information) override { return ErrorType::SUCCESS; };
        ErrorType Create(VkImage&baseImage, TextureViewInformation const &information) override { return ErrorType::SUCCESS; };
        ErrorType CreateFromFile(std::string const &textureFile) override { return ErrorType::SUCCESS; };

    };
} // GPC