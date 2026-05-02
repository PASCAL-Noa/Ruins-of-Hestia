#include "Village/Resource.h"

namespace GPC
{
    void Resource::Add(uint32_t amount)
    {
        if (Count + amount <= Max)
            Count += amount;
        else
            Count = Max;
    }

    void Resource::Remove(uint32_t amount)
    {
        if (Count >= amount)
            Count -= amount;
        else
            Count = 0;
    }

    bool Resource::Has(uint32_t amount) const
    {
        return Count >= amount;
    }

    uint32_t Resource::GetCount() const
    {
        return Count;
    }
}