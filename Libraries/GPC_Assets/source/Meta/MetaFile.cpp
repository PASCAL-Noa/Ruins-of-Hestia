//
// Created by killi on 25/03/2026.
//

#include "../../include/Meta/MetaFile.h"

namespace GPC
{
    bool MetaFile::IsValid() const
    {
        return Magic == META_MAGIC && Version == META_VERSION;
    }

    size_t MetaFile::AssetCount() const
    {
        return Assets.size();
    }

    MetaAsset* MetaFile::FindByName(const std::string& name)
    {
        for (auto& asset : Assets)
        {
            if (asset.Name == name)
                return &asset;
        }
        return nullptr;
    }

    const MetaAsset* MetaFile::FindByName(const std::string& name) const
    {
        for (const auto& asset : Assets)
        {
            if (asset.Name == name)
                return &asset;
        }
        return nullptr;
    }

    std::vector<MetaAsset*> MetaFile::FindByType(MetaAssetType type)
    {
        std::vector<MetaAsset*> result;
        for (auto& asset : Assets)
        {
            if (asset.Type == type)
                result.push_back(&asset);
        }
        return result;
    }

    void MetaFile::EnsureLoaded()
    {
        for (auto& asset : Assets)
            asset.EnsureLoaded();
    }
} // GPC