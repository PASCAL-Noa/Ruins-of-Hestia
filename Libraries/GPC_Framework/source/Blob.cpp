//
// Created by hugoc on 04/03/2026.
//

#include "Blob.h"

#include "Debugger.h"

namespace GPC
{
    Blob::Blob()
    {
        mp_Data = (uint8_t*)malloc(256);
        if (mp_Data == nullptr) {
            GPC_ERROR << "Failed to create Blob";
            return;
        }
        m_DataCapacity = 256;
        m_DataSize = 0;
    }

    Blob::Blob(uint8_t* pData, uint64_t size)
    {
        if (size == 0)
            size = 1;

        mp_Data = (uint8_t*)malloc(size * 2);
        if (mp_Data == nullptr) {
            GPC_ERROR << "Failed to create Blob";
            return;
        }

        memcpy(mp_Data, pData, size);
        m_DataCapacity = 2*size;
        m_DataSize = size;
    }

    Blob::Blob(uint64_t capacity)
    {
        if (capacity == 0) capacity = 1;
        mp_Data = (uint8_t*)malloc(capacity);
        if (mp_Data == nullptr) {
            GPC_ERROR << "Failed to create Blob";
            return;
        }
        m_DataCapacity = capacity;
        m_DataSize = 0;
    }

    Blob::~Blob()
    {
        delete[] mp_Data;
    }

    void Blob::Resize(uint64_t size)
    {
        m_DataSize = size;
        while (m_DataSize >= m_DataCapacity)
        {
            m_DataCapacity *= 2;
        }
        Reserve(m_DataCapacity);
    }

    uint8_t* Blob::Ptr() const
    {
        return mp_Data;
    }

    uint64_t Blob::Size() const
    {
        return m_DataSize;
    }

    void Blob::Reserve(uint64_t size)
    {
        auto* temp = (uint8_t*)realloc(mp_Data, size);
        if (temp == nullptr) {
            GPC_ERROR << "Failed to resize Blob";
            return;
        }
        mp_Data = temp;
        m_DataCapacity = size;
    }
}
