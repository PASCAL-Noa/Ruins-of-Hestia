#ifndef GPC_COLLISION3D_SYSTEM_H
#define GPC_COLLISION3D_SYSTEM_H

#include "ECS_Defines.h"
#include "System.h"
#include "Component.h"

namespace GPC
{
    struct Cell
    {
        std::vector<EntityID> CellEntity;
    };

    struct TraversedCells
    {
        glm::ivec2 CellCoords;
        float minDistanceToNextCell;
    };

    enum class ColliderType : uint8_t
    {
        Player,
        Enemy,
        AllyProjectile,
        EnemyProjectile,
        Props,
        MapBorder,

        Count,
    };

    struct Collision3DComponent : public Component {
        BIND_COMPONENT(COLLISION3D);
        AUTO_COMPONENT_CONSTRUCTOR(Collision3DComponent);

        union {
            glm::vec3 HalfExtends{1, 1, 1};
            float Radius;
        };
        glm::vec3 Offset{0, 0, 0};
        glm::vec3 OldTransformPos{0, 0, 0};
        bool IsSphere = false;
        bool IsTrigger = false;
        bool JustCreated = true;
        bool IsKinematic = true;

        ColliderType CollType;

        std::vector<Cell*> OverlappedCells;
        std::set<EntityID> OverlappedEntity;
        inline bool IsColliding() const { return OverlappedEntity.empty() == false;}
    };


    struct Ray
    {
        Ray(const glm::vec3& origin, const glm::vec3& direction)
            : Origin(origin), Direction(direction)
        {
            InvDirection.x = 1 / direction.x;
            InvDirection.y = 1 / direction.y;
            InvDirection.z = 1 / direction.z;
            maxDistance = 200;
        }
        glm::vec3 Origin;
        glm::vec3 Direction;
        glm::vec3 InvDirection;
        float maxDistance;
    };

    struct RayHit
    {
        glm::vec3 ContactPoint;
        glm::vec3 ContactNormal;
        EntityID HitEntity;
    };

    class Collision3DSystem : public System {
    public:
        BIND_SYSTEM(COLLISION3D);

        inline static bool AllowCollision[(uint8_t)ColliderType::Count][(uint8_t)ColliderType::Count] =
        {
            //Player     Enemy  AllyProjectile EnemyProjectile Props    MapBorder
            {false,     true,       true,      true,       true,       true},  //Player
            {true,      false,      true,       false,       true,      true},  //Enemy
            {true,      true,       false,      false,       true,      true},  //AllyProjectile
            {true,      false,      false,      false,       true,      true},  //EnemyProjectile
            {true,      true,       true,       true,       false,      false}, //Props
            {true,      true,       true,       true,       false,      false}, //MapBorder
        };

        static constexpr glm::u32vec2 GridSize = { 100, 100 };
        glm::u32vec2 CellSize = { 2.5 * GENERAL_SCALE_EXPEDITION, 2.5 * GENERAL_SCALE_EXPEDITION };
        glm::vec2 GridOffset{
            0,
            0
        };

        std::array<std::array<Cell,GridSize.y>, GridSize.x> Grid;
        std::vector<Cell*> ActiveCells;

        std::vector<EntityID> GetEntityToCollide(Collision3DComponent* coll, EntityID id);

        void AddEntityToCell(Transform3D* transform, Collision3DComponent* coll, EntityID id);
        void ResetCells();

        void OnBeginUpdate() override;
        void OnUpdate() override;
        void OnLateUpdate() override;

        void ComputeCollision();
        bool CollisionSphereSphere(const Collision3DComponent* comp1, const Transform3D* transform1, const Collision3DComponent* comp2, const Transform3D* transform2) const;
        bool CollisionSphereAABB(const Collision3DComponent* comp1, const Transform3D* transform1, const Collision3DComponent* comp2, const Transform3D* transform2) const;
        bool CollisionAABBAABB(const Collision3DComponent* comp1, const Transform3D* transform1, const Collision3DComponent* comp2, const Transform3D* transform2) const;

        bool IsColliding(const Collision3DComponent* comp1, const Transform3D* transform1, const Collision3DComponent* comp2, const Transform3D* transform2) const;

        void ResolveCollision(const Collision3DComponent* comp1, Transform3D* transform1, const Collision3DComponent* comp2,  Transform3D* transform2) const;

        void UpdateCollider(Collision3DComponent* comp, EntityID id1) const;

        static float RaySphereCollision(RayHit& hit, const Ray& ray, const Collision3DComponent* collider, Transform3D* transform);
        static float RayAABBCollision(RayHit& hit, const Ray& ray, const Collision3DComponent* collider, Transform3D* transform);
        bool Raycast3D(RayHit& hit, const Ray& ray);

        std::vector<TraversedCells> RaycastGridDDA(const Ray& ray);
    };
}

#endif // GPC_COLLISION3D_SYSTEM_H
