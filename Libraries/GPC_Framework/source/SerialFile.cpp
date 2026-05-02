//
// Created by wgsdp on 12/03/2026.
//

#include "../include/SerialFile.h"

#include "SerialTree.h"

namespace GPC
{
    uint64_t SerialFile::SerializeContext::InternString(const std::string &str)
    {
        const auto it = StringIndex.find(str);
        if (it != StringIndex.end())
            return it->second;

        const uint64_t id = Strings.size();
        StringIndex.emplace(str, id);
        Strings.push_back(str);
        return id;
    }

    void SerialFile::CollectNodes(const SerialTree &node, SerializeContext &context)
    {
        if (node.m_IsMap)
        {
            const uint64_t id = context.MapNodes.size();
            context.MapIDs.emplace(&node, id);
            context.MapNodes.push_back(&node);
            for (const auto& [key, child] : node.m_Map)
            {
                context.InternString(key);
                CollectNodes(*child, context);
            }
        } else
        {
            const uint64_t id = context.DataNodes.size();
            context.DataIDs.emplace(&node, id);
            context.DataNodes.push_back(&node);
        }
    }

    size_t SerialFile::ComputeTotalSize(const SerializeContext &context)
    {
        size_t total = HEADER_SIZE;

        //String Section
        for (const auto& str : context.Strings)
        {
            total += sizeof(uint64_t) * 2 + str.size(); //id + len + str
        }

        //Data index + Map index
        total += sizeof(uint64_t) * context.DataNodes.size();
        total += sizeof(uint64_t) * context.MapNodes.size();

        //Data Section
        for (const SerialTree* node : context.DataNodes)
        {
            total += sizeof(uint64_t) * 2 + node->m_DataSize; //id + byteSize + bytes
        }

        //Map Section
        for (const SerialTree* node : context.MapNodes)
        {
            total += sizeof(uint64_t) * 3; //id + blockSize + childCount
            total += node->m_Map.size() * (sizeof(uint64_t) * 2 + sizeof(uint8_t)); //stringID + childID + type
        }

        return total;
    }

    uint64_t SerialFile::CRC32(const uint8_t *pData, size_t size)
    {
        static uint32_t table[256] = {};
        static bool table_ready = false;
        if (!table_ready)
        {
            for (uint32_t i = 0; i < 256; ++i)
            {
                uint32_t crc = i;
                for (int j = 0; j < 8; ++j)
                {
                    crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1u));
                }
                table[i] = crc;
            }
            table_ready = true;
        }

        uint32_t crc = 0xFFFFFFFFu;
        for (size_t i = 0; i < size; ++i)
        {
            crc = (crc >> 8) ^ table[(crc ^ pData[i]) & 0xFF];
        }

        return static_cast<uint64_t>(crc ^ 0xFFFFFFFFu);
    }

    void SerialFile::WriteHeader(uint8_t *pBuffer, size_t &offset, const Header &header)
    {
        Write(pBuffer, offset, header.Magic);
        Write(pBuffer, offset, header.Version);
        Write(pBuffer, offset, header.Checksum);
        Write(pBuffer, offset, header.StringCount);
        Write(pBuffer, offset, header.DataCount);
        Write(pBuffer, offset, header.MapCount);
        Write(pBuffer, offset, header.StringOffset);
        Write(pBuffer, offset, header.DataIndexOffset);
        Write(pBuffer, offset, header.MapIndexOffset);
        Write(pBuffer, offset, header.DataOffset);
        Write(pBuffer, offset, header.MapOffset);
    }

    SerialFile::Header SerialFile::ReadHeader(uint8_t *pBuffer, size_t &offset)
    {
        Header header{};
        header.Magic = Read<uint32_t>(pBuffer, offset);
        header.Version = Read<uint32_t>(pBuffer, offset);
        header.Checksum = Read<uint64_t>(pBuffer, offset);
        header.StringCount = Read<uint64_t>(pBuffer, offset);
        header.DataCount = Read<uint64_t>(pBuffer, offset);
        header.MapCount = Read<uint64_t>(pBuffer, offset);
        header.StringOffset = Read<uint64_t>(pBuffer, offset);
        header.DataIndexOffset = Read<uint64_t>(pBuffer, offset);
        header.MapIndexOffset = Read<uint64_t>(pBuffer, offset);
        header.DataOffset = Read<uint64_t>(pBuffer, offset);
        header.MapOffset = Read<uint64_t>(pBuffer, offset);

        return header;
    }

    ErrorType SerialFile::ValidateHeader(const Header &header, const uint8_t *pPayload, size_t payloadSize)
    {
        if (header.Magic != MAGIC)
            return ErrorType::INVALID_MAGIC;
        if (header.Version != VERSION)
            return ErrorType::INVALID_VERSION;

        const uint64_t computed = CRC32(pPayload, payloadSize);
        if (computed != header.Checksum)
            return ErrorType::INVALID_CHECKSUM;

        return ErrorType::SUCCESS;
    }

    ErrorType SerialFile::SerializeToBuffer(const SerialTree &tree, uint8_t *pBuffer, size_t bufferSize)
    {
        if (!tree.IsMap())
            return ErrorType::TREE_NOT_MAP;

        SerializeContext context;
        CollectNodes(tree, context);
        size_t offset = 0;

        size_t string_section_size = 0;
        for (const auto& str : context.Strings)
        {
            string_section_size += sizeof(uint64_t) * 2 + str.size();
        }

        const size_t data_index_size = sizeof(uint64_t) * context.DataNodes.size();
        const size_t map_index_size = sizeof(uint64_t) * context.MapNodes.size();

        size_t data_section_size = 0;
        for (const SerialTree* node : context.DataNodes)
        {
            data_section_size += sizeof(uint64_t) * 2 + node->m_DataSize;
        }

        const uint64_t string_offset = HEADER_SIZE;
        const uint64_t data_index_offset = string_offset + string_section_size;
        const uint64_t map_index_offset = data_index_offset + data_index_size;
        const uint64_t data_offset = map_index_offset + map_index_size;
        const uint64_t map_offset = data_offset + data_section_size;

        Header header{};
        header.Magic = MAGIC;
        header.Version = VERSION;
        header.Checksum = 0;
        header.StringCount = context.Strings.size();
        header.DataCount = context.DataNodes.size();
        header.MapCount = context.MapNodes.size();
        header.StringOffset = string_offset;
        header.DataIndexOffset = data_index_offset;
        header.MapIndexOffset = map_index_offset;
        header.DataOffset = data_offset;
        header.MapOffset = map_offset;

        WriteHeader(pBuffer, offset, header);

        for (uint64_t i = 0; i < header.StringCount; ++i)
        {
            const std::string& str = context.Strings[i];
            Write(pBuffer, offset, i);
            Write(pBuffer, offset, str.size());
            memcpy(pBuffer + offset, str.data(), str.size());
            offset += str.size();
        }

        //Data index
        size_t data_block_offset = data_offset;
        for (const SerialTree* node : context.DataNodes)
        {
            Write(pBuffer, offset, (uint64_t)data_block_offset);
            data_block_offset += sizeof(uint64_t) * 2 + node->m_DataSize;
        }

        //Map index
         size_t map_block_offset = map_offset;
        for (const SerialTree* node : context.MapNodes)
        {
            Write(pBuffer, offset, (uint64_t)map_block_offset);
            const uint64_t block_size = sizeof(uint64_t) * 3 + node->m_Map.size() * (sizeof(uint64_t) * 2 + sizeof(uint8_t));
            map_block_offset += block_size;
        }

        //Data Section
        for (uint64_t i = 0; i < context.DataNodes.size(); ++i)
        {
            const SerialTree* node = context.DataNodes[i];
            Write(pBuffer, offset, i);
            Write(pBuffer, offset, node->m_DataSize);
            memcpy(pBuffer + offset, node->mp_Data, node->m_DataSize);
            offset += node->m_DataSize;
        }

        //Map Section
        for (uint64_t i = 0; i < context.MapNodes.size(); ++i)
        {
            const SerialTree* node = context.MapNodes[i];
            const uint64_t child_count = node->m_Map.size();
            const uint64_t block_size = sizeof(uint64_t) * 3 + child_count * (sizeof(uint64_t) * 2 + sizeof(uint8_t));

            Write(pBuffer, offset, i);
            Write(pBuffer, offset, block_size);
            Write(pBuffer, offset, child_count);

            for (const auto& [key, child] : node->m_Map)
            {
                Write(pBuffer, offset, context.StringIndex.at(key));
                if (child->m_IsMap)
                {
                    Write(pBuffer, offset, uint8_t(1));
                    Write(pBuffer, offset, context.MapIDs.at(child));
                }else
                {
                    Write(pBuffer, offset, uint8_t(0));
                    Write(pBuffer, offset, context.DataIDs.at(child));
                }
            }
        }

        //Checksum
        const size_t payload_size = bufferSize - HEADER_SIZE;
        const uint64_t checksum = CRC32(pBuffer + HEADER_SIZE, payload_size);
        memcpy(pBuffer + sizeof(uint32_t) * 2, &checksum, sizeof(uint64_t));

        return ErrorType::SUCCESS;
    }

    ErrorType SerialFile::DeserializeFromBuffer(uint8_t *pBuffer, size_t bufferSize, SerialTree &tree,
                                                            uint64_t rootMapID)
    {
        size_t offset = 0;
        const Header header = ReadHeader(pBuffer, offset);

        const ErrorType err = ValidateHeader(header, pBuffer + HEADER_SIZE, bufferSize - HEADER_SIZE);
        if (err != ErrorType::SUCCESS)
            return err;

        //String Section
        std::vector<std::string> strings(header.StringCount);
        size_t str_offset = header.StringOffset;
        for (uint64_t i = 0; i < header.StringCount; ++i)
        {
            const uint64_t id = Read<uint64_t>(pBuffer, str_offset);
            const uint64_t len = Read<uint64_t>(pBuffer, str_offset);
            if (str_offset + len > bufferSize)
                return ErrorType::FILE_READ_FAILED;
            strings[id].assign(reinterpret_cast<const char*>(pBuffer + str_offset), len);
            str_offset += len;
        }

        //Data Section
        std::vector<Data> data(header.DataCount);
        size_t data_offset = header.DataOffset;
        for (uint64_t i = 0; i < header.DataCount; ++i)
        {
            const uint64_t id = Read<uint64_t>(pBuffer, data_offset);
            const uint64_t size = Read<uint64_t>(pBuffer, data_offset);
            if (data_offset + size > bufferSize)
                return ErrorType::FILE_READ_FAILED;
            data[id] = {pBuffer + data_offset, size};
            data_offset += size;
        }

        //Map Section
        std::vector<Map> maps(header.MapCount);
        std::vector<SerialTree*> mapNodes(header.MapCount, nullptr);
        size_t map_offset = header.MapOffset;
        for (uint64_t i = 0; i < header.MapCount; ++i)
        {
            const uint64_t id = Read<uint64_t>(pBuffer, map_offset);
            Read<uint64_t>(pBuffer, map_offset); //Jump blockSize
            const uint64_t child_count = Read<uint64_t>(pBuffer, map_offset);

            mapNodes[id] = (id == rootMapID) ? &tree : new SerialTree();

            maps[id].Children.resize(child_count);
            for (uint64_t c = 0; c < child_count; ++c)
            {
                maps[id].Children[c].StringID = Read<uint64_t>(pBuffer, map_offset);
                maps[id].Children[c].Type = Read<uint8_t>(pBuffer, map_offset);
                maps[id].Children[c].ChildID = Read<uint64_t>(pBuffer, map_offset);
            }
        }

        for (uint64_t i = 0; i < header.MapCount; ++i)
        {
            SerialTree* map_node = mapNodes[i];
            if (!map_node) continue;

            for (const ChildRef& ref : maps[i].Children)
            {
                const std::string& key = strings[ref.StringID];
                if (ref.Type == 0)
                {
                    SerialTree* data_node = new SerialTree();
                    data_node->Set(
                        const_cast<void*>(static_cast<const void*>(data[ref.ChildID].pData)),
                        data[ref.ChildID].Size,
                        true
                    );
                    map_node->m_Map.emplace(key, data_node);
                }
                else
                {
                    if (!mapNodes[ref.ChildID])
                        return ErrorType::FILE_READ_FAILED;
                    map_node->m_Map.emplace(key, mapNodes[ref.ChildID]);
                }
            }
        }

        return ErrorType::SUCCESS;
    }

    ErrorType SerialFile::SeekToNode(const uint8_t *pBuffer, size_t bufferSize, const Header &header,
        const std::vector<std::string> &keyPath, uint64_t &outMapID)
    {
        std::unordered_map<std::string, uint64_t> string_index;
        size_t str_offset = header.StringOffset;
        for (uint64_t i = 0; i < header.StringCount; ++i)
        {
            const uint64_t id = Read<uint64_t>(pBuffer, str_offset);
            const uint64_t len = Read<uint64_t>(pBuffer, str_offset);
            if (str_offset + len > bufferSize)
                return ErrorType::FILE_READ_FAILED;

            string_index.emplace(std::string(reinterpret_cast<const char*>(pBuffer + str_offset), len), id);
            str_offset += len;
        }

        uint64_t current_map_id = 0;
        for (const std::string& key : keyPath)
        {
            const auto it = string_index.find(key);
            if (it == string_index.end())
                return ErrorType::INVALID_KEY_PATH;
            const uint64_t target_string_id = it->second;

            const size_t map_index_entry = (size_t)header.MapIndexOffset + current_map_id * sizeof(uint64_t);
            if (map_index_entry + sizeof(uint64_t) > bufferSize)
                return ErrorType::FILE_READ_FAILED;
            size_t map_index_offset = map_index_entry;
            const uint64_t block_offset = Read<uint64_t>(pBuffer, map_index_offset);

            size_t offset = block_offset;
            Read<uint64_t>(pBuffer, offset); //id
            Read<uint64_t>(pBuffer, offset); //block_size
            const uint64_t child_count = Read<uint64_t>(pBuffer, offset);

            bool found = false;
            for (uint64_t c = 0; c < child_count; ++c)
            {
                const uint64_t id = Read<uint64_t>(pBuffer, offset);
                const uint64_t child_type = Read<uint8_t>(pBuffer, offset);
                const uint64_t child_id = Read<uint64_t>(pBuffer, offset);

                if (id == target_string_id)
                {
                    if (child_type != 1)
                        return ErrorType::INVALID_KEY_PATH; // Not a MAP
                    current_map_id = child_id;
                    found = true;
                    break;
                }
            }
            if (!found)
                return ErrorType::INVALID_KEY_PATH;
        }

        outMapID = current_map_id;
        return ErrorType::SUCCESS;
    }

    ErrorType SerialFile::Save(const std::string &filepath, const SerialTree &tree)
    {
        if (!tree.m_IsMap)
            return ErrorType::TREE_NOT_MAP;

        SerializeContext context;
        CollectNodes(tree, context);

        const size_t total_size = ComputeTotalSize(context);
        uint8_t* buffer = static_cast<uint8_t*>(malloc(total_size));
        if (!buffer)
            return ErrorType::ALLOC_FAILED;

        const ErrorType ser_err = SerializeToBuffer(tree, buffer, total_size);
        if (ser_err != ErrorType::SUCCESS)
        {
            free(buffer);
            return ser_err;
        }

        std::ofstream file(filepath, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            free(buffer);
            return ErrorType::FILE_NOT_FOUND;
        }

        file.write(reinterpret_cast<const char*>(buffer), (std::streamsize)total_size);
        free(buffer);

        if (!file.good())
            return ErrorType::FILE_WRITE_FAILED;

        return ErrorType::SUCCESS;
    }

    ErrorType SerialFile::Load(const std::string &filepath, SerialTree &tree)
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            return ErrorType::FILE_NOT_FOUND;

        const size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        uint8_t* buffer = static_cast<uint8_t*>(malloc(file_size));
        if (!buffer)
            return ErrorType::ALLOC_FAILED;

        file.read(reinterpret_cast<char*>(buffer), (std::streamsize)file_size);
        if (!file.good())
        {
            free(buffer);
            return ErrorType::FILE_READ_FAILED;
        }

        const ErrorType err = DeserializeFromBuffer(buffer, file_size, tree);
        free(buffer);
        return err;
    }

    ErrorType SerialFile::LoadNode(const std::string &filepath, SerialTree &tree,
        const std::vector<std::string> &keyPath)
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            return ErrorType::FILE_NOT_FOUND;

        const size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        uint8_t* buffer = static_cast<uint8_t*>(malloc(file_size));
        if (!buffer)
            return ErrorType::ALLOC_FAILED;

        file.read(reinterpret_cast<char*>(buffer), (std::streamsize)file_size);
        if (!file.good())
        {
            free(buffer);
            return ErrorType::FILE_READ_FAILED;
        }

        size_t offset = 0;
        const Header header = ReadHeader(buffer, offset);

        const ErrorType val_err = ValidateHeader(header, buffer + HEADER_SIZE, file_size - HEADER_SIZE);
        if (val_err != ErrorType::SUCCESS)
        {
            free(buffer);
            return val_err;
        }

        uint64_t root_map_id = 0;
        const ErrorType seek_err = SeekToNode(buffer, file_size, header, keyPath, root_map_id);
        if (seek_err != ErrorType::SUCCESS)
        {
            free(buffer);
            return seek_err;
        }

        const ErrorType err = DeserializeFromBuffer(buffer, file_size, tree, root_map_id);
        free(buffer);
        return err;
    }
}
