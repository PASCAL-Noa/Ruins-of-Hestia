#pragma once

#include "GPC_Framework.h"
#include <array>
#include <set>

#define MASK_ORIENTATION(o) (RoomMask)(1 << (o))

namespace GPC {
    constexpr uint8_t ROOM_SIZE = 16;

    using TileID = uint8_t;
    constexpr TileID TILE_EMPTY = 0;

    using TileLine = std::array<TileID, ROOM_SIZE>;
    using TileMatrix = std::array<std::array<TileID, ROOM_SIZE>, ROOM_SIZE>;

    using RoomMask = uint8_t;
    constexpr RoomMask MASK_EMPTY = 0;

    using RoomOrientation               = uint8_t;
    constexpr RoomOrientation UP        = 0;
    constexpr RoomOrientation DOWN      = 1;
    constexpr RoomOrientation RIGHT     = 2;
    constexpr RoomOrientation LEFT      = 3;
    constexpr RoomMask MASK_UP          = 1;
    constexpr RoomMask MASK_DOWN        = 2;
    constexpr RoomMask MASK_RIGHT       = 4;
    constexpr RoomMask MASK_LEFT        = 8;

    using RoomRotation                  = uint8_t;
    constexpr RoomRotation ROTATE_0     = 0;
    constexpr RoomRotation ROTATE_90    = 1;
    constexpr RoomRotation ROTATE_180   = 2;
    constexpr RoomRotation ROTATE_270   = 3;

    constexpr uint8_t DIRECTION_COUNT = 4;

    constexpr auto set_ivec2_cmp = [](const glm::ivec2& a, const glm::ivec2& b) {
        return (a.x < b.x) || (a.x == b.x && a.y < b.y);
    };
    using set_ivec2 = std::set<glm::ivec2, decltype(set_ivec2_cmp)>;

    struct RawRoom;

    class Room {
        TileMatrix                             m_Tiles;
        std::array<TileLine, DIRECTION_COUNT>   m_Doors;
        RoomMask                                m_Mask;
        RoomRotation                            m_CurrentRotation;
        bool                                    m_IsActive;
    public:
        Room();

        constexpr void Create(TileMatrix tiles);
	    static Room CreateRoom(TileMatrix tiles);
	    static Room CreateRoomFromRaw(const RawRoom& raw);

        void SetRotation(const RoomRotation rotation) { m_CurrentRotation = rotation; }
        [[nodiscard]] bool IsComplete() const;
        [[nodiscard]] bool IsAnyOpened() const;
        [[nodiscard]] bool IsOpened(RoomOrientation orientation) const;

        [[nodiscard]] bool CanConnect(const Room& room, RoomOrientation orientation) const;

        [[nodiscard]] bool IsLoaded() const { return m_IsActive; }
        [[nodiscard]] TileID Tile(uint8_t column, uint8_t row) const { return Tile(column, row, m_CurrentRotation); }
        [[nodiscard]] TileID Tile(uint8_t column, uint8_t row, RoomRotation rotation) const;
        [[nodiscard]] const TileLine& Door(RoomOrientation orientation) const;
        [[nodiscard]] RoomMask Mask() const { return Mask(m_CurrentRotation); }
        [[nodiscard]] RoomMask Mask(RoomRotation rotation) const;
    };

    struct RawRoom {
        TileMatrix Tiles;
        RoomMask    Mask;
    };

    RoomOrientation ReverseOrientation(RoomOrientation orientation);
    RoomMask ReverseMask(RoomMask mask);

    RoomOrientation RotateOrientation(RoomOrientation orientation, RoomRotation rotation);
    RoomMask RotateMask(RoomMask mask, RoomRotation rotation);

    TileLine Connect(const Room& room1, const Room& room2, RoomOrientation orientation);
    TileLine Connect(const RawRoom& room1, const RawRoom& room2, RoomOrientation orientation);
    TileLine Door(const RawRoom& room, RoomOrientation orientation);
    TileLine EmptyLine();

    RawRoom ToRawRoom(const Room& room, RoomRotation rotation = ROTATE_0);

    static void LoadRoomFromFile(Room& outRoom, std::vector<glm::vec2>, const std::string& path);

	void PrintRoom(Room room, RoomRotation rotation = ROTATE_0);
    void PrintRaw(const RawRoom & raw);
}
