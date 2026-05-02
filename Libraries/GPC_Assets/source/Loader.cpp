#include "Loader.h"

#include "Assets.h"
#include "Stream.h"
#include "stb_image.h"

namespace GPC
{
    bool Loader::LoadTexture(const std::string& filePath, TextureAsset* asset)
    {
        ReadStream stream(filePath.c_str());
        if (stream.GetSize() == 0) return false;

        uint8_t* fileData = nullptr;
        uint64_t fileSize = stream.ReadAll(&fileData);

        if (!fileData) return false;

        int32_t w, h, c;
        uint8_t* decodedPixels = stbi_load_from_memory(
                static_cast<const stbi_uc*>(static_cast<void*>(fileData)),
            static_cast<int>(fileSize),
                &w, &h, &c, RGBA);

        DEBUG_ASSERT(decodedPixels != nullptr, "Failed to load texture from file");

        asset->Width = w;
        asset->Height = h;
        asset->Channels = RGBA;

        uint64_t totalSize = static_cast<uint64_t>(w) * h * RGBA;
        asset->Data.Resize(totalSize);

        memcpy(asset->Data.Ptr(), decodedPixels, totalSize);

        stbi_image_free(decodedPixels);
        delete[] fileData;

        return true;
    }

    void Loader::LoadAtlas(const std::string& filePath, Assets* manager)
    {

    }
}
