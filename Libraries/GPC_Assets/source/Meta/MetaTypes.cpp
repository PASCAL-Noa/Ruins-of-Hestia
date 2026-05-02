//
// Created by killi on 24/03/2026.
//

#include "../../include/Meta/MetaTypes.h"
#include "SerialTree.h"

namespace GPC
{
    MetaDeferredAsset::~MetaDeferredAsset() = default;

    MetaDeferredAsset::MetaDeferredAsset(MetaDeferredAsset&& other) noexcept
        : Type(other.Type)
        , Name(std::move(other.Name))
        , Path(std::move(other.Path))
        , Properties(std::move(other.Properties))
    {
    }

    MetaDeferredAsset& MetaDeferredAsset::operator=(MetaDeferredAsset&& other) noexcept
    {
        if (this != &other)
        {
            Type = other.Type;
            Name = std::move(other.Name);
            Path = std::move(other.Path);
            Properties = std::move(other.Properties);
        }
        return *this;
    }

    const char* MetaAssetTypeToString(MetaAssetType type)
    {
        switch (type)
        {
            case MetaAssetType::UNKNOWN: return "UNKNOWN";
            case MetaAssetType::TEXTURE: return "TEXTURE";
            case MetaAssetType::MESH: return "MESH";
            case MetaAssetType::LIGHT: return "LIGHT";
            case MetaAssetType::SHADER: return "SHADER";
            case MetaAssetType::AUDIO: return "AUDIO";
            case MetaAssetType::SPRITE: return "SPRITE";
            case MetaAssetType::SPRITE_SHEET: return "SPRITE_SHEET";
            case MetaAssetType::ANIMATION: return "ANIMATION";
            case MetaAssetType::GRAPHIC_PROGRAM: return "GRAPHIC_PROGRAM";
            case MetaAssetType::COMPUTE_PROGRAM: return "COMPUTE_PROGRAM";
            case MetaAssetType::TEXTURE_ARRAY: return "TEXTURE_ARRAY";
            case MetaAssetType::SCENE: return "SCENE";
            case MetaAssetType::PREFAB: return "PREFAB";
            case MetaAssetType::META_INCLUDE: return "META_INCLUDE";
            default: return "INVALID";
        }
    }

    const char* MetaErrorToString(MetaError error)
    {
        switch (error)
        {
            case MetaError::SUCCESS: return "SUCCESS";
            case MetaError::FILE_NOT_FOUND: return "FILE_NOT_FOUND";
            case MetaError::INVALID_MAGIC: return "INVALID_MAGIC";
            case MetaError::VERSION_MISMATCH: return "VERSION_MISMATCH";
            case MetaError::PARSE_ERROR: return "PARSE_ERROR";
            case MetaError::ASSET_LOAD_FAILED: return "ASSET_LOAD_FAILED";
            case MetaError::DEPENDENCY_MISSING: return "DEPENDENCY_MISSING";
            case MetaError::UNKNOWN_ASSET_TYPE: return "UNKNOWN_ASSET_TYPE";
            case MetaError::INVALID_CONTEXT: return "INVALID_CONTEXT";
            case MetaError::CYCLE_DETECTED: return "CYCLE_DETECTED";
            default: return "UNKNOWN_ERROR";
        }
    }

    MetaAssetType StringToMetaAssetType(const std::string& str)
    {
        if (str == "TEXTURE") return MetaAssetType::TEXTURE;
        if (str == "MESH") return MetaAssetType::MESH;
        if (str == "LIGHT") return MetaAssetType::LIGHT;
        if (str == "SHADER") return MetaAssetType::SHADER;
        if (str == "AUDIO") return MetaAssetType::AUDIO;
        if (str == "SPRITE") return MetaAssetType::SPRITE;
        if (str == "SPRITE_SHEET") return MetaAssetType::SPRITE_SHEET;
        if (str == "ANIMATION") return MetaAssetType::ANIMATION;
        if (str == "GRAPHIC_PROGRAM") return MetaAssetType::GRAPHIC_PROGRAM;
        if (str == "COMPUTE_PROGRAM") return MetaAssetType::COMPUTE_PROGRAM;
        if (str == "TEXTURE_ARRAY") return MetaAssetType::TEXTURE_ARRAY;
        if (str == "SCENE") return MetaAssetType::SCENE;
        if (str == "PREFAB") return MetaAssetType::PREFAB;
        if (str == "META_INCLUDE" || str == "INCLUDE") return MetaAssetType::META_INCLUDE;
        return MetaAssetType::UNKNOWN;
    }
} // GPC
