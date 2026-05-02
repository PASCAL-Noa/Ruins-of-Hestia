//
// Created by killi on 25/03/2026.
//

#include "../../include/Meta/MetaWriter.h"

namespace GPC
{
    MetaError MetaWriter::Write(const MetaFile& file, const std::string& path)
    {
        SerialTree tree;
        ToTree(file, tree);

        ErrorType err = SerialFile::Save(path, tree);
        if (GPC_FAILED(err))
            return MetaError::ASSET_LOAD_FAILED;

        return MetaError::SUCCESS;
    }

    void MetaWriter::ToTree(const MetaFile& file, SerialTree& outTree)
    {
        uint32_t magic = file.Magic;
        uint32_t version = file.Version;
        outTree["magic"].Set<uint32_t>(&magic, 1);
        outTree["version"].Set<uint32_t>(&version, 1);

        uint32_t count = static_cast<uint32_t>(file.Assets.size());
        outTree["assets"]["count"].Set<uint32_t>(&count, 1);

        for (uint32_t i = 0; i < count; ++i)
        {
            std::string key = std::to_string(i);
            AssetToTree(file.Assets[i], outTree["assets"][key]);
        }
    }

    void MetaWriter::AssetToTree(const MetaAsset& asset, SerialTree& outNode)
    {
        uint8_t typeVal = static_cast<uint8_t>(asset.Type);
        outNode["type"].Set<uint8_t>(&typeVal, 1);

        outNode["name"].Set(const_cast<char*>(asset.Name.data()), asset.Name.size(), true);
        outNode["path"].Set(const_cast<char*>(asset.Path.data()), asset.Path.size(), true);

        if (asset.Optional)
        {
            bool opt = true;
            outNode["optional"].Set<bool>(&opt, 1);
        }

        if (asset.Properties && asset.Properties->ChildCount() > 0)
        {
            CopySerialTree(*asset.Properties, outNode["properties"]);
        }
    }

    void MetaWriter::CopySerialTree(const SerialTree& src, SerialTree& dst)
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
