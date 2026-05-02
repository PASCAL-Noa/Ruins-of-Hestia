#pragma once

#include "GPC_Framework.h"
#include "MapLayout.h"
#include "RoomEditor/RoomMeta.h"

namespace GPC {
    struct GeometryData;

    static constexpr uint32_t TILE_GROUP_COUNT = ROOM_SIZE + 1;

    enum TileState {
        EMPTY,
        FULL
    };

    struct TileGroup {
        union {
            struct {
                uint8_t b00, b10,
                        b01, b11;
            };
            uint8_t b[2][2] = { {EMPTY, EMPTY}, {EMPTY, EMPTY} };
        };

        bool operator < (const TileGroup & other) const {
            return *static_cast<const uint32_t*>(static_cast<const void*>(this)) < *static_cast<const uint32_t*>(static_cast<const void*>(&other));
        }

        TileGroup() = default;
        TileGroup(const uint8_t _00, const uint8_t _10, const uint8_t _01, const uint8_t _11) : b00(_00), b10(_10), b01(_01), b11(_11) { }

        TileGroup Rotated90(uint8_t count = 1) const {
            if (count == 0) return *this;
            return TileGroup{
                b10, b11,
                b00, b01
            }.Rotated90(count - 1);
        }

    };
    struct TileGroupSolution {
        GeometryData* pGeometry;
        float AngleY;
    };
    using TileGroupResolver = std::map<TileGroup, std::vector<TileGroupSolution>>;

    using TileGroupLayout = std::array<std::array<std::array<std::array<TileGroup, TILE_GROUP_COUNT>, TILE_GROUP_COUNT>, MAX_MAP_SIZE>, MAX_MAP_SIZE>;

    TileGroupLayout LayoutToGroupLayout(const MissionData& mission_data);
    std::array<std::array<TileGroup, TILE_GROUP_COUNT>, TILE_GROUP_COUNT> RawRoomToGroupLayout(const RawRoom& raw);
    void LoadTileGroupForOneRoom(
        TileGroupLayout& out,
        const MissionData& mission_data,
        glm::ivec2 room_position,
        const TileLine& up,
        const TileLine& down,
        const TileLine& left,
        const TileLine& right
        );
    // void ApplyFixToGroupLayout(TileGroupLayout& tg_layout, uint32_t x_room, uint32_t y_room, uint32_t x_tile_group, uint32_t y_tile_group);

} // GPC