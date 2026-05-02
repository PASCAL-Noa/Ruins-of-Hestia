#pragma once

#include "Room.h"
#include "RoomEditor/RoomMeta.h"

namespace GPC {
    class MapMask;
    class MapLayout;

    constexpr uint32_t MAX_MAP_SIZE = 7;
    using RoomMatrix = std::array<std::array<RawRoom, MAX_MAP_SIZE>, MAX_MAP_SIZE>;

    class MissionData {
        RoomMatrix m_Rooms;
        std::array<std::array<std::pair<const RoomMeta*, glm::mat2>, MAX_MAP_SIZE>, MAX_MAP_SIZE> mp_Metas;
    public:
        constexpr MissionData() = default;

        void ResolveFor(const MapMask& map_mask, const MapLayout& layout, const Room* pRooms, const RoomMeta* pMetas, uint32_t room_count);

        uint8_t Get(uint32_t roomX, uint32_t roomY, uint32_t tileX, uint32_t tileY) const;
        const RawRoom& GetRoom(uint32_t roomX, uint32_t roomY) const { return m_Rooms[roomX][roomY]; }
        const RoomMeta* GetMeta(uint32_t roomX, uint32_t roomY) const { return mp_Metas[roomX][roomY].first; }
        const glm::mat2& GetRotation(uint32_t roomX, uint32_t roomY) const { return mp_Metas[roomX][roomY].second; }
    };

    void PrintMissionData(const MissionData& mission_data);

} // GPC