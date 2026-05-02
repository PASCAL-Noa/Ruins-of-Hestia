//
// Created by wgsdp on 12/03/2026.
//

#pragma once

#include <cstdint>
#include <cstdlib>
#include <unordered_map>
#include <string>
#include <vector>
#include <assert.h>

namespace GPC
{
    class SerialFile;

    // struct SerialData
    // {
    //     void* mp_Data;
    //     uint64_t m_DataSize;
    //     bool m_IsCopy;
    // };

    class SerialTree
    {
        friend class SerialFile;

        union
        {
            struct
            {
                void* mp_Data;
                uint64_t m_DataSize;
                bool m_IsCopy;
            };
            std::unordered_map<std::string, SerialTree*> m_Map;
        };
        bool m_IsMap;
        bool m_OwnsData;

        void Clear();
        void PrepareForData();
        void AssignData(void* pData, uint64_t byteSize, bool copy);
        void MoveFrom(SerialTree&& tree);

    public:
        SerialTree();
        SerialTree(void* pData, uint64_t byteSize, bool copy = true);
        SerialTree(void* pUniqueData);
        ~SerialTree();

        SerialTree(const SerialTree&) = delete;
        SerialTree& operator=(const SerialTree&) = delete;

        SerialTree(SerialTree&& tree) noexcept;
        SerialTree& operator=(SerialTree&& tree) noexcept;

        void Set(void* pData, uint64_t byteSize, bool copy = true);
        template<typename T>
        void Set(T* pData, uint64_t count = 1, bool copy = true);

        SerialTree& operator[](const std::string& key);
        const SerialTree& operator[](const std::string& key) const;
        template<typename T>
        T* Get() const;

        bool IsMap() const;
        bool IsData() const;

        uint64_t ByteSize() const;
        template<typename T>
        uint64_t Count() const;
        uint64_t ChildCount() const;
        bool HasKey(const std::string& key) const;
        std::vector<std::string> GetKeys() const;
    };

    template<typename T>
    void SerialTree::Set(T *pData, uint64_t count, bool copy)
    {
        Set(static_cast<void*>(pData), count * sizeof(T), copy);
    }

    template<typename T>
    T * SerialTree::Get() const
    {
        assert(!m_IsMap && "[SerialTree] Get() sur un noeud MAP. Utilisez d'abord operator[]");
        assert(m_DataSize > 0 && "[SerialTree] Get(), No data to get");
        assert(m_DataSize >= sizeof(T) && "[SerialTree] Get(), DataSize inférieur à sizeof(T)");
        return static_cast<T*>(mp_Data);
    }

    template<typename T>
    uint64_t SerialTree::Count() const
    {
        assert(!m_IsMap && "[SerialTree] Count() sur un noeud MAP.");
        return m_DataSize / sizeof(T);
    }

}// GPC
