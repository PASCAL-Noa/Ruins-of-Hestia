#pragma once
#include "ECS_Defines.h"
#include "Map Generation/Room.h"

namespace GPC {

    constexpr glm::vec3 PLAYER_SPAWN_EDITOR_SCALE = { 0.25f, 0.25f, 0.25f };
    constexpr glm::vec3 ENNEMI_SPAWN_EDITOR_SCALE = { 0.25f, 0.25f, 0.25f };
    constexpr glm::vec3 RESOURCE_SPAWN_EDITOR_SCALE = { 0.5f, 0.05f, 0.7f };

    struct Spawner { glm::vec2 position; };
    struct PlayerSpawner : public Spawner { };
    struct EnnemiSpawner : public Spawner { };
    struct ResourceSpawner : public Spawner { };

    class RoomMeta {

        struct RoomMetaHeader {
            uint32_t EnnemiSpawnerCount = 0;
            uint32_t ResourceSpawnerCount = 0;
        };

        RawRoom m_Raw{};
        uint32_t m_Width = ROOM_SIZE;
        uint32_t m_Height = ROOM_SIZE;
        std::vector<EnnemiSpawner> m_EnnemiSpawner;
        std::vector<EntityID> m_EnnemiSpawnerID;
        std::vector<ResourceSpawner> m_ResourceSpawner;
        std::vector<EntityID> m_ResourceSpawnerID;
        PlayerSpawner m_PlayerSpawner{ glm::vec2{ ROOM_SIZE * 0.5f, ROOM_SIZE * 0.5f } };
        EntityID m_PlayerSpawnerID{ MAX_ENTITIES };

    public:

        std::string FILE = "";

        bool LoadFromFile(std::string& path);
        bool SaveToFile(std::string& path);

        RawRoom& GetRaw() { return m_Raw; }
        std::vector<EnnemiSpawner>& GetEnnemies() { return m_EnnemiSpawner; }
        std::vector<EntityID>& GetEnnemiesID() { return m_EnnemiSpawnerID; }
        std::vector<ResourceSpawner>& GetResources() { return m_ResourceSpawner; }
        std::vector<EntityID>& GetResourcesID() { return m_ResourceSpawnerID; }
        PlayerSpawner& GetPlayer() { return m_PlayerSpawner; }

        const RawRoom& GetRaw() const { return m_Raw; }
        const std::vector<EnnemiSpawner>& GetEnnemies() const { return m_EnnemiSpawner; }
        const std::vector<ResourceSpawner>& GetResources() const { return m_ResourceSpawner; }
        const PlayerSpawner& GetPlayer() const { return m_PlayerSpawner; }

        void RemoveEnnemiSpawnByID(EntityID id);
        void RemoveResourceSpawnByID(EntityID id);
        EntityID ReplacePlayerSpawn(EntityID id, glm::vec2 pos);
        void AddEnnemiSpawnByID(EntityID id, glm::vec2 pos);
        void AddResourceSpawnByID(EntityID id, glm::vec2 pos);

    };

} // GPC