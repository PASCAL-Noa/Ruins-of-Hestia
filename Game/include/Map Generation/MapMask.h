#pragma once

#include "Map.h"

namespace GPC {

    using BooleanLine = std::array<bool, MAX_MAP_SIZE>;
    using BooleanMatrix = std::array<std::array<bool, MAX_MAP_SIZE>, MAX_MAP_SIZE>;

    class MapMask {
        BooleanMatrix m_MapMask;
    public:
        constexpr MapMask();

        static MapMask CreateAsFull();
        static MapMask CreateAsCircle(float ratio);

        bool Get(int32_t x, int32_t y) const;
    };

    constexpr MapMask::MapMask() {
        for (uint32_t x = 0; x < MAX_MAP_SIZE; ++x) {
            for (uint32_t y = 0; y < MAX_MAP_SIZE; ++y) {
                m_MapMask[x][y] = true;
            }
        }
    }

    void PrintMask(const MapMask& map_mask);

} // GPC