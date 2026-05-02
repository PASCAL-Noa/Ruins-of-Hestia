//
// Created by hugoc on 04/03/2026.
//

#pragma once

#include <cstring>
#include <glm/fwd.hpp>

namespace GPC
{
    class Blob
    {
    public:
        Blob(uint8_t* pData, uint64_t size);
        Blob(uint64_t capacity);
        Blob();
        ~Blob();

        void Resize(uint64_t size);

        [[nodiscard]] uint64_t Size() const;
        [[nodiscard]] uint8_t* Ptr() const;

        template<typename T = uint8_t>
        void Add(const T* pData, uint64_t amount = 1);

        template<typename T = uint8_t>
        void Remove(uint64_t at, uint64_t amount = 1);

        template<typename T = uint8_t>
        void Place(uint64_t at, const T* pData, uint64_t amount = 1);

        template<typename T = uint8_t>
        void Edit(uint64_t at, const T* pData, uint64_t amount = 1);

        template<typename T = uint8_t>
        T* GetAt(uint64_t at);

    private:
        uint8_t* mp_Data;
        uint64_t m_DataSize;
        uint64_t m_DataCapacity;

        void Reserve(uint64_t size);
    };


    //#########################################
    //              TEMPLATE
    //#########################################

    template<typename T>
    inline void Blob::Add(const T* pData, uint64_t amount)
    {
        uint64_t dataSize = sizeof(T) * amount;
        if (m_DataSize + dataSize > m_DataCapacity)
            m_DataCapacity *= 2;

        Reserve(m_DataCapacity);
        memcpy(mp_Data + m_DataSize, pData, dataSize);
        m_DataSize += dataSize;
    }

    template<typename T>
    inline void Blob::Remove(uint64_t at, uint64_t amount)
    {
        uint64_t dataSize = sizeof(T) * amount;

        memcpy(mp_Data + at, mp_Data + at + dataSize, m_DataSize - at - dataSize);

        while (m_DataSize - dataSize < m_DataCapacity / 2) {
            m_DataCapacity /= 2;
        }
        Reserve(m_DataCapacity);
        m_DataSize -= dataSize;
    }

    template<typename T>
    inline void Blob::Place(uint64_t at, const T* pData, uint64_t amount)
    {
        uint64_t dataSize = sizeof(T) * amount;
        if (m_DataSize + dataSize > m_DataCapacity)
            m_DataCapacity *= 2;

        Reserve(m_DataCapacity);
        memcpy(mp_Data + at + dataSize, mp_Data + at, m_DataSize - at);
        memcpy(mp_Data + at, pData, dataSize);
        m_DataSize += dataSize;
    }

    template<typename T>
    inline void Blob::Edit(uint64_t at, const T* pData, uint64_t amount)
    {
        uint64_t dataSize = sizeof(T) * amount;
        if (at + dataSize > m_DataCapacity) {
            m_DataCapacity *= 2;
            m_DataSize = at + dataSize;
        }

        Reserve(m_DataCapacity);
        memcpy(mp_Data + at, pData, dataSize);
    }

    template<typename T>
    inline T* Blob::GetAt(uint64_t at)
    {
        return reinterpret_cast<T*>(mp_Data + at);
    }

}
