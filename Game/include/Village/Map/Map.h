#pragma once
#include "GPC_Framework.h"
#include <unordered_map>
#include <glm/glm.hpp>

#include "ECS_Defines.h"

namespace GPC
{
    class Scene;
    struct Render3DComponent;
    struct Transform3D;

    namespace VillageGroundTilt
    {
        constexpr float YAW_DEG   = 0.0f;
        constexpr float PITCH_DEG = 3.2f;
        constexpr float ROLL_DEG  = -3.2f;

        constexpr float SPRITE_DEPTH_FACTOR     = 0.78f;
        constexpr float SPRITE_LATERAL_FACTOR   = 0.05f;
    }

    struct GridPos
    {
        int X, Y;

        bool operator==(const GridPos& other) const
        {
            return X == other.X && Y == other.Y;
        }

        bool operator<(const GridPos& other) const;

        static GridPos ToGridPos(glm::vec3 worldPos);
    };

    struct GridPosHash
    {
        std::size_t operator()(const GridPos& pos) const
        {
            return std::hash<int>()(pos.X) ^ (std::hash<int>()(pos.Y) << 1);
        }
    };

    class Map
    {
    public:
        Map(Scene* scene, glm::vec2 size);
        ~Map();

        glm::vec2                       GetSize() const;
        bool                            IsSpaceFree(glm::ivec2 position, glm::ivec2 size) const;

        float                           GetGroundYAt(float worldX, float worldZ) const;
        glm::vec2                       ProjectRayToTiltedGround(glm::vec3 rayOrigin, glm::vec3 rayDir) const;

        void                            RegisterBuilding(glm::ivec2 position, glm::ivec2 size, EntityID id);
        void                            RemovePlacedZone(EntityID id);

        EntityID                        GetEntityAt(glm::ivec2 position) const;

        void                            Clear();
        void                            Update();

        void                            SetGridActive(bool active);
        void                            SpawnDecoration(glm::vec2 size, glm::ivec2 position, std::string array, std::string texture, float width = 600, float height = 300);
        void                            SpawnTree(glm::vec2 size, glm::ivec2 position, std::string array, std::string texture);

    private:
        bool                            IsOutOfBounds(const GridPos& pos) const;
        bool                            IsCellOccupied(const GridPos& pos) const;

        Scene*                                                  mp_Scene = nullptr;
        glm::vec2                                               m_Size;

        EntityID                                                m_MapEntity = 0;
        Render3DComponent*                                      mp_Render3DComponentGrid = nullptr;
        Render3DComponent*                                      mp_Render3DComponentGround = nullptr;
        Transform3D*                                            mp_Transform = nullptr;

        std::unordered_map<GridPos, EntityID, GridPosHash>      m_Buildings;
        std::unordered_map<EntityID, EntityID>                  m_BuildingToGrass;
        std::unordered_map<GridPos, Render3DComponent*, GridPosHash>m_Decorations;

        std::vector<Render3DComponent*>                         m_UsedTiles;

        void DrawBackground(uint32_t i, float rotation, bool isX, std::string texture);
    };
}
