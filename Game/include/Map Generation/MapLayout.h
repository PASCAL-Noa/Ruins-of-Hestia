#pragma once

#include "Map.h"
#include <algorithm>

namespace GPC {

    class MapMask;

    using MaskLine = std::array<RoomMask, MAX_MAP_SIZE>;
    using MaskMatrix = std::array<MaskLine, MAX_MAP_SIZE>;

    class MapLayout {
        using DirectionIndex = uint8_t;
        static constexpr uint8_t DIRECTION_INDEX_COUNT = 4;

        MaskMatrix m_Layout;
        glm::ivec2 m_Start{0, 0};
        glm::ivec2 m_End{MAX_MAP_SIZE - 1, MAX_MAP_SIZE - 1};
        void Link(glm::ivec2 a, glm::ivec2 b);

        static void GetMoveExcluded(glm::ivec2 position, const set_ivec2& excluded, const MapMask& map_mask, glm::ivec2& out_direction);
        static void GetMoveIncluded(glm::ivec2 position, const set_ivec2& included, const MapMask& map_mask, glm::ivec2& out_direction);

        int TryResolve(const MapMask& map_mask);
    public:
        constexpr MapLayout();

        void Resolve(const MapMask& map_mask);

        const glm::ivec2& GetStart() const { return m_Start; }
        const glm::ivec2& GetEnd() const { return m_End; }

        RoomMask Get(int32_t x, int32_t y) const;
        const MaskLine& operator[](uint32_t x) const;
    };

    constexpr MapLayout::MapLayout() {
        for (uint32_t x = 0; x < MAX_MAP_SIZE; ++x) {
            for (uint32_t y = 0; y < MAX_MAP_SIZE; ++y) {
                m_Layout[x][y] = MASK_EMPTY;
            }
        }
    }

    void PrintLayout(const MapLayout& layout);

} // GPC