#include "RoomEditor/RoomMeta.h"

#include "Stream.h"

namespace GPC {
    bool RoomMeta::LoadFromFile(std::string &path) {
        RoomMetaHeader header{};
        std::ifstream stream{path, std::ios::binary};
        if (stream.is_open() == false) return false;

        stream.read(static_cast<char*>(static_cast<void*>(&header)), sizeof(RoomMetaHeader));

        stream.read(static_cast<char*>(static_cast<void*>(&m_Raw)), sizeof(RawRoom));

        stream.read(static_cast<char*>(static_cast<void*>(&m_PlayerSpawner)), sizeof(PlayerSpawner));

        m_EnnemiSpawner.resize(header.EnnemiSpawnerCount);
        stream.read(static_cast<char*>(static_cast<void*>(m_EnnemiSpawner.data())), sizeof(EnnemiSpawner) * header.EnnemiSpawnerCount);

        m_ResourceSpawner.resize(header.ResourceSpawnerCount);
        stream.read(static_cast<char*>(static_cast<void*>(m_ResourceSpawner.data())), sizeof(ResourceSpawner) * header.ResourceSpawnerCount);

        stream.close();
        return true;
    }

    bool RoomMeta::SaveToFile(std::string &path) {
        RoomMetaHeader header{};
        header.EnnemiSpawnerCount = m_EnnemiSpawner.size();
        header.ResourceSpawnerCount = m_ResourceSpawner.size();

        std::ofstream stream{path, std::ios::binary};
        if (stream.is_open() == false) return false;

        stream.write(static_cast<char*>(static_cast<void*>(&header)), sizeof(RoomMetaHeader));
        stream.write(static_cast<char*>(static_cast<void*>(&m_Raw)), sizeof(RawRoom));
        stream.write(static_cast<char*>(static_cast<void*>(&m_PlayerSpawner)), sizeof(PlayerSpawner));
        stream.write(static_cast<char*>(static_cast<void*>(m_EnnemiSpawner.data())), sizeof(EnnemiSpawner) * m_EnnemiSpawner.size());
        stream.write(static_cast<char*>(static_cast<void*>(m_ResourceSpawner.data())), sizeof(ResourceSpawner) * m_ResourceSpawner.size());

        stream.close();
        return true;
    }

    EntityID RoomMeta::ReplacePlayerSpawn(EntityID id, glm::vec2 pos) {
        auto temp = m_PlayerSpawnerID;
        m_PlayerSpawnerID = id;
        m_PlayerSpawner = {pos};
        return temp;
    }

    void RoomMeta::RemoveEnnemiSpawnByID(EntityID id) {
        for (uint32_t i = 0; i < m_EnnemiSpawnerID.size(); ++i) {
            if (m_EnnemiSpawnerID[i] == id) {
                m_EnnemiSpawnerID[i] = m_EnnemiSpawnerID[m_EnnemiSpawnerID.size() - 1];
                m_EnnemiSpawner[i] = m_EnnemiSpawner[m_EnnemiSpawnerID.size() - 1];
                m_EnnemiSpawnerID.resize(m_EnnemiSpawnerID.size() - 1);
                m_EnnemiSpawner.resize(m_EnnemiSpawner.size() - 1);
            }
        }

    }

    void RoomMeta::AddEnnemiSpawnByID(EntityID id, glm::vec2 pos) {
        m_EnnemiSpawnerID.push_back(id);
        m_EnnemiSpawner.push_back({pos});
    }

    void RoomMeta::RemoveResourceSpawnByID(EntityID id) {
        for (uint32_t i = 0; i < m_ResourceSpawnerID.size(); ++i) {
            if (m_ResourceSpawnerID[i] == id) {
                m_ResourceSpawnerID[i] = m_ResourceSpawnerID[m_ResourceSpawnerID.size() - 1];
                m_ResourceSpawner[i] = m_ResourceSpawner[m_ResourceSpawnerID.size() - 1];
                m_ResourceSpawnerID.resize(m_ResourceSpawnerID.size() - 1);
                m_ResourceSpawner.resize(m_ResourceSpawner.size() - 1);
            }
        }
    }

    void RoomMeta::AddResourceSpawnByID(EntityID id, glm::vec2 pos) {
        m_ResourceSpawnerID.push_back(id);
        m_ResourceSpawner.push_back({pos});
    }
} // GPC