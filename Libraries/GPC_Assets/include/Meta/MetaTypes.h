//
// Created by killi on 24/03/2026.
//

#ifndef RUINS_OF_HESTIA_METATYPES_H
#define RUINS_OF_HESTIA_METATYPES_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace GPC
{
    class RenderWindow;
    class SerialTree;

    constexpr uint32_t META_VERSION = 1;
    constexpr uint32_t META_MAGIC = 0x4D455441;  // "META"

    enum class MetaAssetType : uint8_t
    {
        UNKNOWN = 0,

        // Phase 1 - Core
        TEXTURE = 1,
        MESH = 2,
        LIGHT = 3,
        SHADER = 4,
        AUDIO = 5,

        //  Phase 2 - Composite
        SPRITE = 6,
        SPRITE_SHEET = 7,
        ANIMATION = 8,
        GRAPHIC_PROGRAM = 9,
        COMPUTE_PROGRAM = 10,
        TEXTURE_ARRAY = 11,

        // Phase 3 - High-level
        SCENE = 100,
        PREFAB = 101,
        META_INCLUDE = 102,
    };

    enum class MetaError : uint8_t
    {
        SUCCESS = 0,
        FILE_NOT_FOUND,
        INVALID_MAGIC,
        VERSION_MISMATCH,
        PARSE_ERROR,
        ASSET_LOAD_FAILED,
        DEPENDENCY_MISSING,
        UNKNOWN_ASSET_TYPE,
        INVALID_CONTEXT,
        CYCLE_DETECTED,
    };

    const char* MetaAssetTypeToString(MetaAssetType type);
    const char* MetaErrorToString(MetaError error);
    MetaAssetType StringToMetaAssetType(const std::string& str);

    struct MetaDeferredAsset
    {
        MetaAssetType Type = MetaAssetType::UNKNOWN;
        std::string Name;
        std::string Path;
        std::unique_ptr<SerialTree> Properties;

        MetaDeferredAsset() = default;
        ~MetaDeferredAsset();
        MetaDeferredAsset(MetaDeferredAsset&& other) noexcept;
        MetaDeferredAsset& operator=(MetaDeferredAsset&& other) noexcept;
        MetaDeferredAsset(const MetaDeferredAsset&) = delete;
        MetaDeferredAsset& operator=(const MetaDeferredAsset&) = delete;
    };

    struct MetaRuntimeContext
    {
        RenderWindow* Window = nullptr;
    };

}

#endif //RUINS_OF_HESTIA_METATYPES_H