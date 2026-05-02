#include "Map Generation/MapMask.h"

namespace GPC {

    MapMask MapMask::CreateAsFull() {
        MapMask mm;
        for (uint32_t x = 0; x < MAX_MAP_SIZE; ++x) {
            for (uint32_t y = 0; y < MAX_MAP_SIZE; ++y) {
                mm.m_MapMask[x][y] = true;
            }
        }
        return mm;
    }

    MapMask MapMask::CreateAsCircle(float ratio) {
        MapMask mm;
        for (uint32_t x = 0; x < MAX_MAP_SIZE; ++x) {
            for (uint32_t y = 0; y < MAX_MAP_SIZE; ++y) {
                const float fx = (static_cast<float>(x) / static_cast<float>(MAX_MAP_SIZE - 1)) - 0.5f;
                const float fy = (static_cast<float>(y) / static_cast<float>(MAX_MAP_SIZE - 1)) - 0.5f;
                const float sq_distance = fx * fx + fy * fy;
                mm.m_MapMask[x][y] = sq_distance < ratio * ratio * 0.25f;
            }
        }
        return mm;
    }

    bool MapMask::Get(int32_t x, int32_t y) const {
        if (x >= 0 || y >= 0 || x < MAX_MAP_SIZE || y < MAX_MAP_SIZE) return m_MapMask[x][y];
        return false;
    }

    void PrintMask(const MapMask &map_mask)
    {
        for (uint32_t x = 0; x < MAX_MAP_SIZE + 4; ++x)
        {
            std::cout << "-";
        }
        std::cout << std::endl;

        for (uint32_t y = 0; y < MAX_MAP_SIZE; ++y)
        {
            std::cout << "| ";
            for (uint32_t x = 0; x < MAX_MAP_SIZE; ++x)
            {
                std::cout << (map_mask.Get(x, y) ? "O" : " ");
            }
            std::cout << " |" << std::endl;
        }

        for (uint32_t x = 0; x < MAX_MAP_SIZE + 4; ++x)
        {
            std::cout << "-";
        }
        std::cout << std::endl;
    }

} // GPC