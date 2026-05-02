#include "Map Generation/Map.h"

#include "Map Generation/MapMask.h"
#include "Map Generation/MapLayout.h"

namespace GPC {

    void MissionData::ResolveFor(const MapMask &map_mask, const MapLayout &layout, const Room *pRooms, const RoomMeta *pMetas, uint32_t room_count) {
        using RotationAndRoom = std::pair<RoomRotation, std::pair<const Room*, const RoomMeta*>>;
        std::map<uint8_t, std::vector<RotationAndRoom>> preprocess_table;
        for (int i = 0; i < room_count; ++i)
        {
            const Room* pRoom = pRooms + i;
            const RoomMeta* pMeta = pMetas + i;
            for (uint8_t rotation = 0; rotation < DIRECTION_COUNT; ++rotation)
            {
                auto mask = pRooms[i].Mask(rotation);
                if (preprocess_table.contains(mask))
                {
                    preprocess_table[mask].push_back( { rotation, { pRoom, pMeta } } );
                }
                else {
                    preprocess_table.emplace(mask, std::vector<RotationAndRoom>{ { rotation, { pRoom, pMeta } } });
                }
            }
        }

        for (int32_t y = 0; y < MAX_MAP_SIZE; ++y)
        {
            for (int32_t x = 0; x < MAX_MAP_SIZE; ++x)
            {
                auto m = layout.Get(x, y);
                if (m == MASK_EMPTY) continue;
                auto& solutions = preprocess_table[m];
                RotationAndRoom& rr = solutions[rand() % solutions.size()];
                m_Rooms[x][y] = ToRawRoom(*rr.second.first, rr.first);
                constexpr static glm::mat2 MATRICES[4] = {
                    {  0, -1,  1,  0 },
                    {  1,  0,  0,  1 },
                    {  0,  1, -1,  0 },
                    { -1,  0,  0, -1 },
                };
                mp_Metas[x][y] = { rr.second.second, MATRICES[rr.first] };
            }
        }
    }

    uint8_t MissionData::Get(uint32_t roomX, uint32_t roomY, uint32_t tileX, uint32_t tileY) const {
        return m_Rooms[roomX][roomY].Tiles[tileX][tileY];
    }

    void PrintMissionData(const MissionData &mission_data) {
        constexpr static char CHARACTER[] = { ' ', 'n', '0' };
        for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
            for (uint32_t y_tile = 0; y_tile < ROOM_SIZE; ++y_tile) {
                for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
                    for (uint32_t x_tile = 0; x_tile < ROOM_SIZE; ++x_tile) {
                        auto i = mission_data.Get(x_room, y_room, x_tile, y_tile);
                        if (i < 3) std::cout << CHARACTER[i];
                        else std::cout << " ";
                    }
                }
                std::cout << std::endl;
            }
        }
    }
} // GPC