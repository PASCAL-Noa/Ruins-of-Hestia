//
// Created by killi on 25/03/2026.
//

#ifndef RUINS_OF_HESTIA_METAFILE_H
#define RUINS_OF_HESTIA_METAFILE_H

#include "MetaAsset.h"
#include <vector>

namespace GPC
{
    struct MetaFile
    {
        uint32_t Magic = META_MAGIC;
        uint32_t Version = META_VERSION;
        std::string SourcePath;
        std::vector<MetaAsset> Assets;

        bool IsValid() const;
        size_t AssetCount() const;

        MetaAsset* FindByName(const std::string& name);
        const MetaAsset* FindByName(const std::string& name) const;
        std::vector<MetaAsset*> FindByType(MetaAssetType type);

        void EnsureLoaded();
    };
} // GPC

#endif //RUINS_OF_HESTIA_METAFILE_H