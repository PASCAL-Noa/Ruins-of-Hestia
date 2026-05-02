//
// Created by wgsdp on 12/03/2026.
//

#include "SerialTree.h"

#include <cstring>

namespace GPC
{
    void SerialTree::Clear()
    {
        if (m_IsMap)
        {
            for (auto& [key, node] : m_Map)
            {
                delete node;
            }
            m_Map.~unordered_map<std::string, SerialTree*>();
        } else if (m_OwnsData && mp_Data)
        {
            free(mp_Data);
            mp_Data = nullptr;
        }
    }

    void SerialTree::PrepareForData()
    {
        if (m_IsMap)
        {
            assert(m_Map.empty() && "[SerialTree] set() sur un noeud MAP non vide.");
            m_Map.~unordered_map<std::string, SerialTree*>();
            m_IsMap = false;
            m_OwnsData = false;
            mp_Data = nullptr;
            m_DataSize = 0;
        } else if (m_OwnsData && mp_Data)
        {
            free(mp_Data);
            mp_Data = nullptr;
        }
    }

    void SerialTree::AssignData(void *pData, uint64_t byteSize, bool copy)
    {
        if (copy)
        {
            mp_Data = malloc(byteSize);
            assert(mp_Data && "[SerialTree] AssignData - malloc failed.");
            std::memcpy(mp_Data, pData, byteSize);
            m_OwnsData = true;
        } else
        {
            mp_Data = pData;
            m_OwnsData = false;
        }
        m_DataSize = byteSize;
        m_IsMap = false;
    }

    void SerialTree::MoveFrom(SerialTree &&tree)
    {
        m_IsMap = tree.m_IsMap;
        m_OwnsData = tree.m_OwnsData;
        if (m_IsMap)
        {
            new (&m_Map) std::unordered_map<std::string, SerialTree*>(std::move(tree.m_Map));
        } else
        {
            mp_Data = tree.mp_Data;
            m_DataSize = tree.m_DataSize;
            tree.mp_Data = nullptr;
            tree.m_DataSize = 0;
            tree.m_OwnsData = false;
        }
    }

    SerialTree::SerialTree()
        : m_IsMap(true), m_OwnsData(false)
    {
        new (&m_Map) std::unordered_map<std::string, SerialTree*>();
    }

    SerialTree::SerialTree(void *pData, uint64_t byteSize, bool copy) : SerialTree() {
        Set(pData, byteSize, copy);
    }

    SerialTree::SerialTree(void *pUniqueData) : SerialTree(pUniqueData, 1, true) {

    }

    SerialTree::~SerialTree()
    {
        Clear();
    }

    SerialTree::SerialTree(SerialTree &&tree) noexcept
        : m_IsMap(false), m_OwnsData(false)
    {
        MoveFrom(std::move(tree));
    }

    SerialTree & SerialTree::operator=(SerialTree &&tree) noexcept
    {
        if (this != &tree)
        {
            if (m_IsMap && tree.m_IsMap)
            {
                for (auto& [key, node] : m_Map) delete node;
                m_Map = std::move(tree.m_Map);
            } else
            {
                Clear();
                MoveFrom(std::move(tree));
            }
        }
        return *this;
    }

    void SerialTree::Set(void *pData, uint64_t byteSize, bool copy)
    {
        PrepareForData();
        AssignData(pData, byteSize, copy);
    }

    SerialTree & SerialTree::operator[](const std::string &key)
    {
        assert(m_IsMap && "[SerialTree] operator[] sur un noeud DATA. Utilisez Get()");
        auto [it, inserted] = m_Map.emplace(key, nullptr);
        if (inserted)
        {
            it->second = new SerialTree();
        }
        return *it->second;
    }

    const SerialTree & SerialTree::operator[](const std::string &key) const
    {
        assert(m_IsMap && "[SerialTree] operator[] sur un noeud DATA.");
        auto it = m_Map.find(key);
        assert(it != m_Map.end() && "[SerialTree] Clé introuvable");
        return *it->second;
    }

    bool SerialTree::IsMap() const
    {
        return m_IsMap;
    }

    bool SerialTree::IsData() const
    {
        return !m_IsMap;
    }

    uint64_t SerialTree::ByteSize() const
    {
        assert(!m_IsMap && "[SerialTree] byteSize() sur un noeud MAP.");
        return m_DataSize;
    }

    uint64_t SerialTree::ChildCount() const
    {
        assert(m_IsMap && "[SerialTree] ChildCount() sur un noeud DATA.");
        return m_Map.size();
    }

    bool SerialTree::HasKey(const std::string &key) const
    {
        assert(m_IsMap && "[SerialTree] HasKey() sur un noeud DATA.");
        return m_Map.count(key) != 0;
    }

    std::vector<std::string> SerialTree::GetKeys() const
    {
        assert(m_IsMap && "[SerialTree] GetKeys() sur un noeud DATA.");
        std::vector<std::string> keys;
        keys.reserve(m_Map.size());
        for (const auto& [key, _] : m_Map)
            keys.push_back(key);
        return keys;
    }
}// GPC
