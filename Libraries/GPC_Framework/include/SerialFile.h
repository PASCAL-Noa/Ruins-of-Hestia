//
// Created by wgsdp on 12/03/2026.
//

#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

#include "ErrorType.h"

namespace GPC
{
    class SerialTree;

    class SerialFile
    {
    private:
        static constexpr uint32_t MAGIC = 0x47504300; //GPC\0
        static constexpr uint32_t VERSION = 1;
        static constexpr size_t HEADER_SIZE = sizeof(uint32_t) * 2 + sizeof(uint64_t) * 9; //[magic + version] + [checksum + 3 counts + 5 offsets]

        struct Header
        {
            uint32_t Magic;
            uint32_t Version;
            uint64_t Checksum;      //CRC32
            uint64_t StringCount;
            uint64_t DataCount;
            uint64_t MapCount;
            uint64_t StringOffset;  //offset absolu depuis début du fichier
            uint64_t DataIndexOffset;
            uint64_t MapIndexOffset;
            uint64_t DataOffset;
            uint64_t MapOffset;
        };
        struct ChildRef
        {
            uint64_t StringID;
            uint8_t Type;       //0 = DATA, 1 = MAP
            uint64_t ChildID;
        };
        struct Map
        {
            std::vector<ChildRef> Children;
        };
        struct Data
        {
            const uint8_t* pData;
            uint64_t Size;
        };
        struct SerializeContext
        {
            std::unordered_map<std::string, uint64_t> StringIndex;
            std::vector<std::string> Strings;

            std::vector<const SerialTree*> DataNodes;
            std::vector<const SerialTree*> MapNodes;

            std::unordered_map<const SerialTree*, uint64_t> DataIDs;
            std::unordered_map<const SerialTree*, uint64_t> MapIDs;

            uint64_t InternString(const std::string& str);
        };

        static void CollectNodes(const SerialTree& node, SerializeContext& context);
        static size_t ComputeTotalSize(const SerializeContext& context);
        static uint64_t CRC32(const uint8_t* pData, size_t size);

        template<typename T>
        static void Write(uint8_t* pBuffer, size_t& offset, const T& value);
        template<typename T>
        static T Read(const uint8_t* pBuffer, size_t& offset);

        static void WriteHeader(uint8_t* pBuffer, size_t& offset, const Header& header);
        static Header ReadHeader(uint8_t* pBuffer, size_t& offset);
        static ErrorType ValidateHeader(const Header& header, const uint8_t* pPayload, size_t payloadSize);
        static ErrorType SerializeToBuffer(const SerialTree& tree, uint8_t* pBuffer, size_t bufferSize);
        static ErrorType DeserializeFromBuffer(uint8_t *pBuffer, size_t bufferSize, SerialTree &tree, uint64_t rootMapID = 0);
        static ErrorType SeekToNode(const uint8_t* pBuffer, size_t bufferSize, const Header& header, const std::vector<std::string>& keyPath, uint64_t& outMapID);

    public:
        static ErrorType Save(const std::string& filepath, const SerialTree& tree);
        static ErrorType Load(const std::string& filepath, SerialTree& tree);
        static ErrorType LoadNode(const std::string& filepath, SerialTree& tree, const std::vector<std::string>& keyPath);
    };

    template<typename T>
    void SerialFile::Write(uint8_t *pBuffer, size_t &offset, const T &value)
    {
        memcpy(pBuffer + offset, &value, sizeof(T));
        offset += sizeof(T);
    }

    template<typename T>
    T SerialFile::Read(const uint8_t *pBuffer, size_t &offset)
    {
        T value;
        memcpy(&value, pBuffer + offset, sizeof(T));
        offset += sizeof(T);
        return value;
    }
}
