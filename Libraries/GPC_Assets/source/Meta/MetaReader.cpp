//
// Created by killi on 25/03/2026.
//

#include "../../include/Meta/MetaReader.h"

namespace GPC
{
    MetaError MetaReader::Read(const std::string& path, MetaFile& outFile)
    {
        SerialTree tree;
        ErrorType err = SerialFile::Load(path, tree);
        if (GPC_FAILED(err))
            return MetaError::FILE_NOT_FOUND;

        MetaError result = ReadFromTree(tree, outFile);
        outFile.SourcePath = path;
        return result;
    }

    MetaError MetaReader::ReadFromTree(const SerialTree& tree, MetaFile& outFile)
    {
        if (!tree.HasKey("magic") || !tree.HasKey("version"))
            return MetaError::PARSE_ERROR;

        outFile.Magic = *tree["magic"].Get<uint32_t>();
        outFile.Version = *tree["version"].Get<uint32_t>();

        if (outFile.Magic != META_MAGIC)
            return MetaError::INVALID_MAGIC;
        if (outFile.Version != META_VERSION)
            return MetaError::VERSION_MISMATCH;

        if (!tree.HasKey("assets"))
            return MetaError::SUCCESS;

        const SerialTree& assets = tree["assets"];
        if (!assets.HasKey("count"))
            return MetaError::SUCCESS;

        uint32_t count = *assets["count"].Get<uint32_t>();
        outFile.Assets.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            std::string key = std::to_string(i);
            if (!assets.HasKey(key))
                continue;

            MetaAsset asset;
            MetaError err = ParseAsset(assets[key], asset);
            if (err != MetaError::SUCCESS)
                return err;

            outFile.Assets.push_back(std::move(asset));
        }

        return MetaError::SUCCESS;
    }

    MetaError MetaReader::ParseAsset(const SerialTree& node, MetaAsset& outAsset)
    {
        if (!node.HasKey("type") || !node.HasKey("name") || !node.HasKey("path"))
            return MetaError::PARSE_ERROR;

        outAsset.Type = static_cast<MetaAssetType>(*node["type"].Get<uint8_t>());

        const SerialTree& nameNode = node["name"];
        if (nameNode.ByteSize() > 0)
            outAsset.Name.assign(nameNode.Get<char>(), nameNode.ByteSize());
        else
            outAsset.Name.clear();

        const SerialTree& pathNode = node["path"];
        if (pathNode.ByteSize() > 0)
            outAsset.Path.assign(pathNode.Get<char>(), pathNode.ByteSize());
        else
            outAsset.Path.clear();

        if (node.HasKey("optional"))
            outAsset.Optional = *node["optional"].Get<bool>();

        if (node.HasKey("properties"))
        {
            outAsset.Properties = std::make_unique<SerialTree>();
            CopySerialTree(node["properties"], *outAsset.Properties);
        }

        return MetaError::SUCCESS;
    }

    void MetaReader::CopySerialTree(const SerialTree& src, SerialTree& dst)
    {
        if (src.IsData())
        {
            if (src.ByteSize() > 0)
            {
                dst.Set(const_cast<void*>(static_cast<const void*>(src.Get<char>())),
                        src.ByteSize(), true);
            }
        }
        else if (src.IsMap() && src.ChildCount() > 0)
        {
            for (const auto& key : src.GetKeys())
            {
                CopySerialTree(src[key], dst[key]);
            }
        }
    }
} // GPC
