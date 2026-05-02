//
// Created by killi on 25/03/2026.
//

#ifndef RUINS_OF_HESTIA_METAREADER_H
#define RUINS_OF_HESTIA_METAREADER_H

#include "MetaFile.h"
#include "SerialFile.h"
#include <string>

namespace GPC
{
    class MetaReader
    {
    public:
        MetaReader() = default;
        ~MetaReader() = default;

        MetaReader(const MetaReader&) = delete;
        MetaReader& operator=(const MetaReader&) = delete;
        MetaReader(MetaReader&&) noexcept = default;
        MetaReader& operator=(MetaReader&&) noexcept = default;

        MetaError Read(const std::string& path, MetaFile& outFile);
        MetaError ReadFromTree(const SerialTree& tree, MetaFile& outFile);
        static void CopySerialTree(const SerialTree& src, SerialTree& dst);

    private:
        MetaError ParseAsset(const SerialTree& node, MetaAsset& outAsset);
    };
}// GPC

#endif //RUINS_OF_HESTIA_METAREADER_H
