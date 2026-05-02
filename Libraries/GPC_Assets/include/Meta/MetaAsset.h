//
// Created by killi on 25/03/2026.
//

#ifndef RUINS_OF_HESTIA_METAASSET_H
#define RUINS_OF_HESTIA_METAASSET_H

#include "MetaTypes.h"
#include "SerialTree.h"
#include <memory>

namespace GPC {

     struct MetaAsset {
         MetaAssetType Type = MetaAssetType::UNKNOWN;
         std::string Name;
         std::string Path;
         std::unique_ptr<SerialTree> Properties;
         bool Optional = false;
         bool Loaded = true;

         MetaAsset();
         ~MetaAsset();

         MetaAsset(MetaAsset&& other) noexcept;
         MetaAsset& operator=(MetaAsset&& other) noexcept;
         MetaAsset(const MetaAsset&) = delete;
         MetaAsset& operator=(const MetaAsset&) = delete;

         bool IsValid() const;
         bool HasProperty(const std::string& key) const;

         void EnsureLoaded();

         std::string ToString() const;
     };
} // GPC

#endif //RUINS_OF_HESTIA_METAASSET_H