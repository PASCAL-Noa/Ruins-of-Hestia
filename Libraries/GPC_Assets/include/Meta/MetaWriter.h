//
// Created by killi on 25/03/2026.
//

#ifndef RUINS_OF_HESTIA_METAWRITER_H
#define RUINS_OF_HESTIA_METAWRITER_H

#include "MetaFile.h"
#include "SerialFile.h"
#include <string>

namespace GPC
{
    class MetaWriter
    {
    public:
        MetaWriter() = default;
        ~MetaWriter() = default;

        MetaWriter(const MetaWriter&) = delete;
        MetaWriter& operator=(const MetaWriter&) = delete;
        MetaWriter(MetaWriter&&) noexcept = default;
        MetaWriter& operator=(MetaWriter&&) noexcept = default;

        MetaError Write(const MetaFile& file, const std::string& path);
        void ToTree(const MetaFile& file, SerialTree& outTree);

    private:
        void AssetToTree(const MetaAsset& asset, SerialTree& outNode);
        static void CopySerialTree(const SerialTree& src, SerialTree& dst);
    };
} // GPC

#endif //RUINS_OF_HESTIA_METAWRITER_H
