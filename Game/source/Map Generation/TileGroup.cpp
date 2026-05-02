#include "Map Generation/TileGroup.h"

namespace GPC {

    TileGroupLayout LayoutToGroupLayout(
        const MissionData& mission_data
        )   {
        TileGroupLayout layout{};
        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                TileLine up     = y_room > 0                ? Connect(mission_data.GetRoom(x_room, y_room), mission_data.GetRoom(x_room, y_room - 1), UP) : EmptyLine();
                TileLine down   = y_room < MAX_MAP_SIZE - 1 ? Connect(mission_data.GetRoom(x_room, y_room), mission_data.GetRoom(x_room, y_room + 1), DOWN) : EmptyLine();
                TileLine left   = x_room > 0                ? Connect(mission_data.GetRoom(x_room, y_room), mission_data.GetRoom(x_room - 1, y_room), LEFT) : EmptyLine();
                TileLine right  = x_room < MAX_MAP_SIZE - 1 ? Connect(mission_data.GetRoom(x_room, y_room), mission_data.GetRoom(x_room + 1, y_room), RIGHT) : EmptyLine();

                LoadTileGroupForOneRoom(
                    layout,
                    mission_data,
                    {x_room, y_room},
                    up,
                    down,
                    left,
                    right
                );
            }
        }
        return layout;
    }

    std::array<std::array<TileGroup, TILE_GROUP_COUNT>, TILE_GROUP_COUNT> RawRoomToGroupLayout(const RawRoom &raw) {

        std::array<std::array<TileGroup, TILE_GROUP_COUNT>, TILE_GROUP_COUNT> layout{};

        for (int32_t x_tile = 0; x_tile < ROOM_SIZE; ++x_tile) {
            for (int32_t y_tile = 0; y_tile < ROOM_SIZE; ++y_tile) {

                auto current = raw.Tiles[x_tile][y_tile];

                layout[x_tile][y_tile].b[1][1] = current;
                layout[x_tile + 1][y_tile].b[0][1] = current;
                layout[x_tile][y_tile + 1].b[1][0] = current;
                layout[x_tile + 1][y_tile + 1].b[0][0] = current;

                // top
                if (y_tile == 0) {
                    layout[x_tile][y_tile].b[1][0] = current;
                    layout[x_tile + 1][y_tile].b[0][0] = current;
                }
                // left
                if (x_tile == 0) {
                    layout[x_tile][y_tile].b[0][1] = current;
                    layout[x_tile][y_tile + 1].b[0][0] = current;
                }
                // bottom
                if (y_tile == ROOM_SIZE - 1) {
                    layout[x_tile][y_tile + 1].b[1][1] = current;
                    layout[x_tile + 1][y_tile + 1].b[0][1] = current;
                }
                // right
                if (x_tile == ROOM_SIZE - 1) {
                    layout[x_tile + 1][y_tile].b[1][1] = current;
                    layout[x_tile + 1][y_tile + 1].b[1][0] = current;
                }

            }
        }

        return layout;
    }

    void LoadTileGroupForOneRoom(
        TileGroupLayout& out,
        const MissionData &mission_data,
        glm::ivec2 room_position,
        const TileLine& up,
        const TileLine& down,
        const TileLine& left,
        const TileLine& right
        ) {

        auto& dual_grid = out[room_position.x][room_position.y];

        for (int32_t x_tile = 0; x_tile < ROOM_SIZE; ++x_tile) {
            for (int32_t y_tile = 0; y_tile < ROOM_SIZE; ++y_tile) {

                auto current = mission_data.Get(room_position.x, room_position.y, x_tile, y_tile);

                dual_grid[x_tile][y_tile].b[1][1] = current;
                dual_grid[x_tile + 1][y_tile].b[0][1] = current;
                dual_grid[x_tile][y_tile + 1].b[1][0] = current;
                dual_grid[x_tile + 1][y_tile + 1].b[0][0] = current;

                // top
                if (y_tile == 0) {
                    dual_grid[x_tile][y_tile].b[1][0] = up[x_tile];
                    dual_grid[x_tile + 1][y_tile].b[0][0] = up[x_tile];
                }
                // left
                if (x_tile == 0) {
                    dual_grid[x_tile][y_tile].b[0][1] = left[y_tile];
                    dual_grid[x_tile][y_tile + 1].b[0][0] = left[y_tile];
                }
                // bottom
                if (y_tile == ROOM_SIZE - 1) {
                    dual_grid[x_tile][y_tile + 1].b[1][1] = down[x_tile];
                    dual_grid[x_tile + 1][y_tile + 1].b[0][1] = down[x_tile];
                }
                // right
                if (x_tile == ROOM_SIZE - 1) {
                    dual_grid[x_tile + 1][y_tile].b[1][1] = right[y_tile];
                    dual_grid[x_tile + 1][y_tile + 1].b[1][0] = right[y_tile];
                }

            }
        }

    }
} // GPC