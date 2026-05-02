#include "Map Generation/Room.h"


namespace GPC {
    Room::Room() :
        m_IsActive(false),
        m_CurrentRotation(ROTATE_0),
        m_Mask(MASK_EMPTY)
    {
        for (int x = 0; x < ROOM_SIZE; ++x) {
            for (int y = 0; y < ROOM_SIZE; ++y) {
                m_Tiles[x][y] = TILE_EMPTY;
            }
            m_Doors[UP   ][x]    = TILE_EMPTY;
            m_Doors[DOWN ][x]    = TILE_EMPTY;
            m_Doors[RIGHT][x]    = TILE_EMPTY;
            m_Doors[LEFT ][x]    = TILE_EMPTY;
        }
    }


    constexpr void Room::Create(TileMatrix tiles) {
        for (int x = 0; x < ROOM_SIZE; ++x) {
            for (int y = 0; y < ROOM_SIZE; ++y) {
                TileID currentID = tiles[y][x];
                m_Tiles[x][y] = currentID;
                if (x == 0) {
                    m_Doors[LEFT][y] = currentID;
                    if (currentID != TILE_EMPTY)
                        m_Mask |= MASK_LEFT;
                }
                if (y == 0) {
                    m_Doors[UP][x] = currentID;
                    if (currentID != TILE_EMPTY)
                        m_Mask |= MASK_UP;
                }
                if (x == ROOM_SIZE - 1) {
                    m_Doors[RIGHT][y] = currentID;
                    if (currentID != TILE_EMPTY)
                        m_Mask |= MASK_RIGHT;
                }
                if (y == ROOM_SIZE - 1) {
                    m_Doors[DOWN][x] = currentID;
                    if (currentID != TILE_EMPTY)
                        m_Mask |= MASK_DOWN;
                }
            }
        }
        m_IsActive = true;
    }

    Room Room::CreateRoom(TileMatrix tiles) {
        Room room{};
        room.Create(tiles);
        return room;
    }

    Room Room::CreateRoomFromRaw(const RawRoom &raw) {
        Room room{};
        TileMatrix mat;
        for (uint32_t i = 0; i < ROOM_SIZE; ++i) {
            for (uint32_t j = 0; j < ROOM_SIZE; ++j) {
                mat[i][j] = raw.Tiles[i][j];
            }
        }
        room.Create(mat);
        return room;
    }

    bool Room::IsComplete() const {
        glm::ivec2 first{};
        for (int32_t x = 0; x < ROOM_SIZE; ++x) {
            for (int32_t y = 0; y < ROOM_SIZE; ++y) {

                if (m_Tiles[x][y])
                {
                    first = { x, y };
                    goto FOUND;
                }
            }
        }
        return false;
    FOUND:

        set_ivec2 done{};
        std::queue<glm::ivec2> next{};
        next.push(first);

        const int AXIS_COUNT = 8;
        constexpr static glm::ivec2 AXIS[AXIS_COUNT] = {
            { 0, -1 },
            { 1, -1 },
            { 1, 0 },
            { 1, 1 },
            { 0, 1 },
            { -1, 1},
            { -1, 0},
            { -1, -1}
        };

        while (!next.empty())
        {
            glm::ivec2 current = next.front();
            next.pop();


            for (auto axe : AXIS)
            {
                glm::ivec2 tile = current + axe;

                if (
                    tile.x >= 0 &&
                    tile.y >= 0 &&
                    tile.x < ROOM_SIZE &&
                    tile.y < ROOM_SIZE)
                {
                    if (m_Tiles[tile.x][tile.y] && done.contains(tile) == false) {
                        done.insert(tile);
                        next.push(tile);
                    }
                }
            }
        }

        for (int32_t x = 0; x < ROOM_SIZE; ++x) {
            for (int32_t y = 0; y < ROOM_SIZE; ++y) {

                if (m_Tiles[x][y])
                {
                    if (done.contains({ x, y }) == false) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    bool Room::IsAnyOpened() const {
        return IsOpened(UP) || IsOpened(DOWN) || IsOpened(LEFT) || IsOpened(RIGHT);
    }

    bool Room::IsOpened(RoomOrientation orientation) const {
        return (Mask() & MASK_ORIENTATION(orientation));
    }

    bool Room::CanConnect(const Room &room, RoomOrientation orientation) const {
        return (m_Mask & MASK_ORIENTATION(orientation)) && (room.m_Mask & MASK_ORIENTATION(ReverseOrientation(orientation)));
    }

    TileID Room::Tile(uint8_t column, uint8_t row, RoomRotation rotation) const {
        constexpr static struct ROTATION_MATRIX { float m[2][2]; } MATRICES[4] = {
            { { {  1,  0 }, {  0,  1 } } },
            { { {  0,  1 }, { -1,  0 } } },
            { { { -1,  0 }, {  0, -1 } } },
            { { {  0, -1 }, {  1,  0 } } }
        };
        const ROTATION_MATRIX& m = MATRICES[rotation];

        constexpr float HALF_ROOM_SIZE = static_cast<float>(ROOM_SIZE - 1) * 0.5f;
        const float c = static_cast<float>(column) - HALF_ROOM_SIZE;
        const float r = static_cast<float>(row) - HALF_ROOM_SIZE;

        float iX = c * m.m[0][0] + r * m.m[0][1];
        float iY = c * m.m[1][0] + r * m.m[1][1];

        iX += HALF_ROOM_SIZE;
        iY += HALF_ROOM_SIZE;

        return m_Tiles[static_cast<int>(iX)][static_cast<int>(iY)];
    }

    const TileLine & Room::Door(RoomOrientation orientation) const {
        return m_Doors[RotateOrientation(orientation, m_CurrentRotation)];
    }

    RoomMask Room::Mask(RoomRotation rotation) const {
        return RotateMask(m_Mask, rotation);
    }

    RoomOrientation ReverseOrientation(RoomOrientation orientation) {
        orientation -= ((orientation % 2) * 2 - 1);
        return orientation;
    }

    RoomMask ReverseMask(RoomMask mask) {
        return ((mask << 1) & 0b1010) | ((mask >> 1) & 0b0101);
    }

    RoomOrientation RotateOrientation(RoomOrientation orientation, RoomRotation rotation) {
        // UP => RIGHT		| 0 => 2 | 2
        // DOWN => LEFT		| 1 => 3 | 2
        // RIGHT => DOWN	| 2 => 1 | -1
        // LEFT => UP		| 3 => 0 | -3
        if (rotation == ROTATE_0) return orientation;
        switch (orientation) {
            case UP: return RotateOrientation(orientation + 2, rotation - 1);
            case DOWN: return RotateOrientation(orientation + 1, rotation - 1);
            case RIGHT: return RotateOrientation(orientation - 1, rotation - 1);
            case LEFT: return RotateOrientation(orientation - 3, rotation - 1);
            default: return orientation;
        }
    }

    RoomMask RotateMask(RoomMask mask, RoomRotation rotation) {
        // UP => RIGHT		| 0 => 2 | 2
        // DOWN => LEFT		| 1 => 3 | 2
        // RIGHT => DOWN	| 2 => 1 | -1
        // LEFT => UP		| 3 => 0 | -3
        if (rotation == ROTATE_0) return mask;
        RoomMask mask_up    = (mask & MASK_UP) << 2;
        RoomMask mask_down  = (mask & MASK_DOWN) << 2;
        RoomMask mask_right = (mask & MASK_RIGHT) >> 1;
        RoomMask mask_left  = (mask & MASK_LEFT) >> 3;
        return RotateMask(mask_up | mask_down | mask_right | mask_left, rotation - 1);
    }

    TileLine Connect(const Room &room1, const Room &room2, RoomOrientation orientation) {
        TileLine row = EmptyLine();
        TileLine r1_row = room1.Door(orientation);
        TileLine r2_row = room2.Door(ReverseOrientation(orientation));

        int first = 0;
        for (int i = 0; i < ROOM_SIZE; ++i)
        {
            if (r1_row[i]) { first = i; break; }
            if (r2_row[i]) { first = i; break; }
        }

        int last = ROOM_SIZE - 1;
        for (int i = 0; i < ROOM_SIZE; ++i)
        {
            if (r1_row[i]) { last = i; continue; }
            if (r2_row[i]) { last = i; continue; }
        }

        for (int i = first; i <= last; ++i)
        {
            row[i] = true;
        }

        return row;

    }

    TileLine Connect(const RawRoom &room1, const RawRoom &room2, RoomOrientation orientation) {
        TileLine row = EmptyLine();
        TileLine r1_row = Door(room1, orientation);
        TileLine r2_row = Door(room2, ReverseOrientation(orientation));;

        int first = 0;
        int last = ROOM_SIZE - 1;

        for (int i = 0; i < ROOM_SIZE; ++i)
        {
            if (r1_row[i]) { first = i; goto END_FIRST; }
            if (r2_row[i]) { first = i; goto END_FIRST; }
        }

        goto EMPTY_CONNECTION;
    END_FIRST:

        for (int i = 0; i < ROOM_SIZE; ++i)
        {
            if (r1_row[i]) { last = i; continue; }
            if (r2_row[i]) { last = i; continue; }
        }

        for (int i = first; i <= last; ++i)
        {
            row[i] = true;
        }

        return row;

EMPTY_CONNECTION:
        return EmptyLine();
    }

    TileLine Door(const RawRoom &room, RoomOrientation orientation) {
        TileLine row = EmptyLine();

        switch (orientation) {

                case UP: {
                    for (uint32_t i = 0; i < ROOM_SIZE; ++i) {
                        row[i] = room.Tiles[i][0];
                    }
                    return row;
                }

                case DOWN: {
                    for (uint32_t i = 0; i < ROOM_SIZE; ++i) {
                        row[i] = room.Tiles[i][ROOM_SIZE - 1];
                    }
                    return row;
                }

                case LEFT: {
                    for (uint32_t i = 0; i < ROOM_SIZE; ++i) {
                        row[i] = room.Tiles[0][i];
                    }
                    return row;
                }

                case RIGHT: {
                    for (uint32_t i = 0; i < ROOM_SIZE; ++i) {
                        row[i] = room.Tiles[ROOM_SIZE - 1][i];
                    }
                    return row;
                }

                default: return row;
        }

    }

    TileLine EmptyLine() {
        TileLine line{};
        for (int i = 0; i < ROOM_SIZE; ++i)
        {
            line[i] = TILE_EMPTY;
        }
        return line;
    }

    RawRoom ToRawRoom(const Room &room, RoomRotation rotation) {
        RawRoom raw{};
        if (room.IsLoaded() == false) {
            for (int32_t x = 0; x < ROOM_SIZE; ++x) {
                for (int32_t y = 0; y < ROOM_SIZE; ++y) {
                    raw.Tiles[x][y] = TILE_EMPTY;
                }
            }
            raw.Mask = MASK_EMPTY;
            return raw;
        }
        for (int32_t x = 0; x < ROOM_SIZE; ++x) {
            for (int32_t y = 0; y < ROOM_SIZE; ++y) {
                raw.Tiles[x][y] = room.Tile(x, y, rotation);
            }
        }
        raw.Mask = room.Mask(rotation);
        return raw;
    }

    void PrintRoom(Room room, RoomRotation rotation) {
        static char CHARACTER[2] = {
            '.', // EMPTY
            'O'  // FULL
        };

        std::cout << "  ";
        for (int i = 0; i < ROOM_SIZE; ++i) {
            std::cout << CHARACTER[room.Door(UP)[i]];
        }
        std::cout << std::endl;

        for (int y = 0; y < ROOM_SIZE; ++y) {
            std::cout << CHARACTER[room.Door(LEFT)[y]] << " ";

            for (int x = 0; x < ROOM_SIZE; ++x) {
                std::cout << CHARACTER[room.Tile(x, y, rotation)];
            }
            std::cout << " " << CHARACTER[room.Door(RIGHT)[y]] << std::endl;
        }

        std::cout << "  ";
        for (int i = 0; i < ROOM_SIZE; ++i) {
            std::cout << CHARACTER[room.Door(DOWN)[i]];
        }
        std::cout << std::endl;
    }

    void PrintRaw(const RawRoom & raw) {
        static char CHARACTER[2] = {
            '.', // EMPTY
            'O'  // FULL
        };
        std::cout <<
                "{\n" <<
                "\tUP = " << (raw.Mask & MASK_UP ? "OPEN" : "CLOSE") << "\n" <<
                "\tDOWN = " << (raw.Mask & MASK_DOWN ? "OPEN" : "CLOSE") << "\n" <<
                "\tRIGHT = " << (raw.Mask & MASK_RIGHT ? "OPEN" : "CLOSE") << "\n" <<
                "\tLEFT = " << (raw.Mask & MASK_LEFT ? "OPEN" : "CLOSE") << "\n" <<
                "}" << std::endl;
        for (int y = 0; y < ROOM_SIZE; ++y) {
            for (int x = 0; x < ROOM_SIZE; ++x) {
                std::cout << CHARACTER[raw.Tiles[x][y]];
            }
            std::cout << std::endl;
        }
    }
}
