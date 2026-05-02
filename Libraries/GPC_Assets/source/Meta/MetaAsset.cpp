//
// Created by killi on 25/03/2026.
//

#include "../../include/Meta/MetaAsset.h"
#include <sstream>

namespace GPC
{
    MetaAsset::MetaAsset() : Properties(std::make_unique<SerialTree>()) {}

    MetaAsset::~MetaAsset() = default;

    MetaAsset::MetaAsset(MetaAsset&& other) noexcept
        : Type(other.Type)
        , Name(std::move(other.Name))
        , Path(std::move(other.Path))
        , Properties(std::move(other.Properties))
        , Optional(other.Optional)
        , Loaded(other.Loaded)
    {
        other.Type = MetaAssetType::UNKNOWN;
        other.Loaded = false;
    }

    MetaAsset& MetaAsset::operator=(MetaAsset&& other) noexcept {
        if (this != &other) {
            Type = other.Type;
            Name = std::move(other.Name);
            Path = std::move(other.Path);
            Properties = std::move(other.Properties);
            Optional = other.Optional;
            Loaded = other.Loaded;
            other.Type = MetaAssetType::UNKNOWN;
            other.Loaded = false;
        }
        return *this;
    }

    bool MetaAsset::IsValid() const {
        return Type != MetaAssetType::UNKNOWN && !Name.empty();
    }

    bool MetaAsset::HasProperty(const std::string& key) const {
        return Properties && Properties->HasKey(key);
    }

    void MetaAsset::EnsureLoaded() {
        // TODO: impl lazy loading
    }

    std::string MetaAsset::ToString() const {
        std::ostringstream ss;
        ss << "MetaAsset { "
           << "Type=" << MetaAssetTypeToString(Type) << ", "
           << "Name=\"" << Name << "\", "
           << "Path=\"" << Path << "\""
           << (Optional ? ", Optional" : "")
           << " }";
        return ss.str();
    }
} // GPC