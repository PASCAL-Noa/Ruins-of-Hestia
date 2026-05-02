#include "Collision3DSystem.h"
#include "Scene.h"

namespace GPC
{
    std::vector<EntityID> Collision3DSystem::GetEntityToCollide(Collision3DComponent* coll, EntityID id)
    {
        std::vector<EntityID> entities;

        for (auto cell : coll->OverlappedCells)
        {
            for (auto ent : cell->CellEntity)
            {
                if (id != ent)
                    entities.push_back(ent);
            }
        }

        return entities;
    }

    void Collision3DSystem::AddEntityToCell(Transform3D* transform, Collision3DComponent* coll, const EntityID id)
    {
        const glm::vec3 boundingBoxPos = transform->LocalTransform.GetPosition() + coll->Offset;

        int minX = static_cast<int>(glm::floor((boundingBoxPos.x - coll->HalfExtends.x - GridOffset.x) / CellSize.x + GridSize.x * 0.5f));
        int minZ = static_cast<int>(glm::floor((boundingBoxPos.z - coll->HalfExtends.z - GridOffset.y)  / CellSize.y + GridSize.y * 0.5f));

        int maxX = static_cast<int>(glm::floor((boundingBoxPos.x + coll->HalfExtends.x - GridOffset.x) / CellSize.x + GridSize.x * 0.5f));
        int maxZ = static_cast<int>(glm::floor((boundingBoxPos.z + coll->HalfExtends.z - GridOffset.y)  / CellSize.y + GridSize.y * 0.5f));

        for (int x = minX; x <= maxX; ++x )
        {
            if (x < 0 || x >= GridSize.x) continue;
            for (int z = minZ; z <= maxZ; ++z )
            {
                if (z < 0 || z >= GridSize.y) continue;

                if ( Grid[x][z].CellEntity.empty())
                    ActiveCells.push_back(&Grid[x][z]);

                Grid[x][z].CellEntity.push_back(id);
                coll->OverlappedCells.push_back(&Grid[x][z]);
            }
        }
    }

    void Collision3DSystem::ResetCells()
    {
        for (int i = 0; i < GridSize.x; ++i)
        {
            for (int j = 0; j < GridSize.y; ++j)
            {
                Grid[i][j].CellEntity.clear();
                ActiveCells.clear();
            }
        }
    }

    void Collision3DSystem::OnBeginUpdate()
    {
        System::OnBeginUpdate();

        const CBlockPtr<Collision3DComponent> collision_component_block = m_Ctx.pScene->GetComponentBlock<Collision3DComponent>();
        Collision3DComponent* collision_components = collision_component_block->ComponentData();
        const EntityID* entities = collision_component_block->EntitiesData();
        const int64_t entityCount = static_cast<int64_t>(collision_component_block->GetEntityCount());

        ResetCells();
        for (int i = 0; i < entityCount; ++i)
        {
            Transform3D* transform = m_Ctx.pScene->GetComponent<Transform3D>(entities[i]);
            Collision3DComponent* comp1 = &collision_components[i];
            comp1->OverlappedCells.clear();
            comp1->OverlappedEntity.clear();

            if (comp1->JustCreated)
            {
                comp1->OldTransformPos = transform->LocalTransform.GetPosition();
                comp1->JustCreated = false;
            }

            AddEntityToCell(transform, comp1, entities[i]);
        }
    }

    void Collision3DSystem::OnUpdate()
    {
        System::OnUpdate();

        const CBlockPtr<Collision3DComponent> collision_component_block = m_Ctx.pScene->GetComponentBlock<Collision3DComponent>();
        Collision3DComponent* collision_components = collision_component_block->ComponentData();
        const EntityID* entities = collision_component_block->EntitiesData();
        const int64_t entityCount = static_cast<int64_t>(collision_component_block->GetEntityCount());

        for (int i = 0; i < entityCount; ++i)
        {
            UpdateCollider(&collision_components[i], entities[i]);
        }

        for (int i = 0; i < entityCount; ++i)
        {
            EntityID CurrentEntity = entities[i];
            Collision3DComponent* compA = &collision_components[i];
            Transform3D* transform_3dA = m_Ctx.pScene->GetComponent<Transform3D>(CurrentEntity);
            std::vector<Cell*>& cells = collision_components[i].OverlappedCells;

            for (auto overlappedCell : cells)
            {
                if (overlappedCell->CellEntity.size() < 2)
                    continue;

                for (auto ent : overlappedCell->CellEntity)
                {
                    Collision3DComponent* compB = m_Ctx.pScene->GetComponent<Collision3DComponent>(ent);
                    Transform3D* transform_3dB = m_Ctx.pScene->GetComponent<Transform3D>(ent);

                    if (CurrentEntity >= ent) continue;

                    if (AllowCollision[(uint8_t)compA->CollType][(uint8_t)compB->CollType] == false)
                        continue;

                    if (IsColliding(compA, transform_3dA, compB, transform_3dB) == false)
                        continue;

                    compB->OverlappedEntity.insert(CurrentEntity);
                }
            }
        }
    }

    void Collision3DSystem::OnLateUpdate()
    {
        System::OnLateUpdate();

        const CBlockPtr<Collision3DComponent> collision_component_block = m_Ctx.pScene->GetComponentBlock<Collision3DComponent>();
        Collision3DComponent* collision_components = collision_component_block->ComponentData();
        const EntityID* entities = collision_component_block->EntitiesData();
        const int64_t entityCount = static_cast<int64_t>(collision_component_block->GetEntityCount());

        ComputeCollision();
        for (int i = 0; i < entityCount; ++i)
        {
            collision_components[i].OldTransformPos =  m_Ctx.pScene->GetComponent<Transform3D>(entities[i])->LocalTransform.GetPosition();
        }

    }

    void Collision3DSystem::ComputeCollision()
    {
        const CBlockPtr<Collision3DComponent> collision_component_block = m_Ctx.pScene->GetComponentBlock<Collision3DComponent>();
        Collision3DComponent* collision_components = collision_component_block->ComponentData();
        const EntityID* entities = collision_component_block->EntitiesData();
        const int64_t entityCount = static_cast<int64_t>(collision_component_block->GetEntityCount());

        for (int i = 0; i < entityCount; ++i)
        {
            Collision3DComponent* compA = &collision_components[i];
            Transform3D* transform_3dA = m_Ctx.pScene->GetComponent<Transform3D>(entities[i]);

            for (auto B : compA->OverlappedEntity)
            {
                Collision3DComponent* compB =  m_Ctx.pScene->GetComponent<Collision3DComponent>(B);
                Transform3D* transform_3dB = m_Ctx.pScene->GetComponent<Transform3D>(B);

                glm::vec3 collision_point = (transform_3dA->GetWorldPosition() + transform_3dB->GetWorldPosition()) * 0.5f;

                BehaviorCollision3DContext ctx1{};
                ctx1.pScene = m_Ctx.pScene;
                ctx1.pClock = m_Ctx.pClock;
                ctx1.Collided.EID = B;
                ctx1.Collided.pCollider = compB;
                ctx1.Collided.pTransform = transform_3dB;
                ctx1.CollisionPoint = collision_point;

                BehaviorCollision3DContext ctx2{};
                ctx2.pScene = m_Ctx.pScene;
                ctx2.pClock = m_Ctx.pClock;
                ctx2.Collided.EID = compA->GetEntityID();
                ctx2.Collided.pCollider = compA;
                ctx2.Collided.pTransform = transform_3dA;
                ctx2.CollisionPoint = collision_point;

                m_Ctx.pScene->ReportCollision3D(compA->GetEntityID(), B, &ctx1, &ctx2);

                if (compA->IsTrigger || compB->IsTrigger)
                    continue;

                ResolveCollision(compA, transform_3dA, compB, transform_3dB);
            }
        }
    }

    bool Collision3DSystem::CollisionSphereSphere(const Collision3DComponent* comp1, const Transform3D* transform1,
        const Collision3DComponent* comp2, const Transform3D* transform2) const
    {
        const glm::vec3 boundingBoxPos1 = transform1->LocalTransform.GetPosition() + comp1->Offset;
        const glm::vec3 boundingBoxPos2 = transform2->LocalTransform.GetPosition() + comp2->Offset;

        const float distX = boundingBoxPos2.x - boundingBoxPos1.x;
        const float distY = boundingBoxPos2.y - boundingBoxPos1.y;
        const float distZ = boundingBoxPos2.z - boundingBoxPos1.z;

        const float radiiSum = comp1->Radius + comp2->Radius;
        const float sqDistance = distX * distX + distY * distY + distZ * distZ;

        return sqDistance <= radiiSum * radiiSum;
    }

    bool Collision3DSystem::CollisionSphereAABB(const Collision3DComponent* comp1, const Transform3D* transform1,
        const Collision3DComponent* comp2, const Transform3D* transform2) const
    {
        const glm::vec3 boundingBoxPos1 = transform1->LocalTransform.GetPosition() + comp1->Offset;
        const glm::vec3 boundingBoxPos2 = transform2->LocalTransform.GetPosition() + comp2->Offset;

        const float closestBoxX = glm::max(boundingBoxPos2.x - comp2->HalfExtends.x, glm::min(boundingBoxPos1.x, boundingBoxPos2.x + comp2->HalfExtends.x));
        const float closestBoxY = glm::max(boundingBoxPos2.y - comp2->HalfExtends.y, glm::min(boundingBoxPos1.y, boundingBoxPos2.y + comp2->HalfExtends.y));
        const float closestBoxZ = glm::max(boundingBoxPos2.z - comp2->HalfExtends.z, glm::min(boundingBoxPos1.z, boundingBoxPos2.z + comp2->HalfExtends.z));

        const float sqDistance =
            (closestBoxX - boundingBoxPos1.x) * (closestBoxX - boundingBoxPos1.x) +
            (closestBoxY - boundingBoxPos1.y) * (closestBoxY - boundingBoxPos1.y) +
            (closestBoxZ - boundingBoxPos1.z) * (closestBoxZ - boundingBoxPos1.z);

        return sqDistance <= comp1->Radius * comp1->Radius;
    }

    bool Collision3DSystem::CollisionAABBAABB(const Collision3DComponent* comp1, const Transform3D* transform1,
        const Collision3DComponent* comp2, const Transform3D* transform2) const
    {
        const glm::vec3 boundingBoxPos1 = transform1->LocalTransform.GetPosition() + comp1->Offset;
        const glm::vec3 boundingBoxPos2 = transform2->LocalTransform.GetPosition() + comp2->Offset;

        return (
            boundingBoxPos1.x - comp1->HalfExtends.x <= boundingBoxPos2.x + comp2->HalfExtends.x &&
            boundingBoxPos1.x + comp1->HalfExtends.x >= boundingBoxPos2.x - comp2->HalfExtends.x &&

            boundingBoxPos1.y - comp1->HalfExtends.y <= boundingBoxPos2.y + comp2->HalfExtends.y &&
            boundingBoxPos1.y + comp1->HalfExtends.y >= boundingBoxPos2.y - comp2->HalfExtends.y &&

            boundingBoxPos1.z - comp1->HalfExtends.z <= boundingBoxPos2.z + comp2->HalfExtends.z &&
            boundingBoxPos1.z + comp1->HalfExtends.z >= boundingBoxPos2.z - comp2->HalfExtends.z
        );
    }

    bool Collision3DSystem::IsColliding(const Collision3DComponent* comp1, const Transform3D* transform1,
        const Collision3DComponent* comp2, const Transform3D* transform2) const
    {
        if (comp1 == nullptr || comp2 == nullptr) return false;

        if (comp1->IsSphere)
        {
            if (comp2->IsSphere)
                return CollisionSphereSphere(comp1, transform1, comp2, transform2);

            return CollisionSphereAABB(comp1, transform1, comp2, transform2);
        }
        if (comp2->IsSphere)
            return CollisionSphereAABB(comp2, transform2, comp1, transform1);

        return CollisionAABBAABB(comp1, transform1, comp2, transform2);
    }

    void Collision3DSystem::ResolveCollision(const Collision3DComponent* comp1, Transform3D* transform1,
        const Collision3DComponent* comp2, Transform3D* transform2) const
    {
        if (comp1->IsKinematic)
            transform1->LocalTransform.SetPosition(comp1->OldTransformPos);
        if (comp2->IsKinematic)
            transform2->LocalTransform.SetPosition(comp2->OldTransformPos);
    }


    void Collision3DSystem::UpdateCollider(Collision3DComponent* comp, const EntityID id1) const
    {
        auto transform = m_Ctx.pScene->GetComponent<Transform3D>(id1);
        comp->HalfExtends =  transform->LocalTransform.GetScale() * 0.5f;
    }

    float Collision3DSystem::RaySphereCollision(RayHit& hit, const Ray& ray, const Collision3DComponent* collider, Transform3D* transform)
    {
        // reference : https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html

        float t0;
        float t1;
        glm::vec3 center =  transform->LocalTransform.GetPosition() + collider->Offset;
        // Geometric solution
        glm::vec3 L = center - ray.Origin;
        float tca = glm::dot(L, ray.Direction);
        if (tca < 0) return -1.f;

        float d2 = glm::dot(L, L) - tca * tca;
        if (d2 > collider->Radius * collider->Radius) return -1.f;
        float thc = sqrt(collider->Radius * collider->Radius - d2);
        t0 = tca - thc;
        t1 = tca + thc;
        if (t0 > t1) std::swap(t0, t1);

        if (t0 < 0)
        {
            t0 = t1; // If t0 is negative, let's use t1 instead.
            if (t0 < 0) return -1.f; // Both t0 and t1 are negative.
        }

        hit.ContactPoint = ray.Origin + ray.Direction * t0;
        hit.ContactNormal = glm::normalize(center - hit.ContactPoint);
        hit.HitEntity = collider->GetEntityID();

        return t0;
    }

    float Collision3DSystem::RayAABBCollision(RayHit& hit, const Ray& ray, const Collision3DComponent* collider,
        Transform3D* transform)
    {
        // reference : https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html

        glm::vec3 boxCenter = transform->LocalTransform.GetPosition() + collider->Offset;
        float minX = boxCenter.x - collider->HalfExtends.x;
        float minY = boxCenter.y - collider->HalfExtends.y;
        float minZ = boxCenter.z - collider->HalfExtends.z;

        float maxX = boxCenter.x + collider->HalfExtends.x;
        float maxY = boxCenter.y + collider->HalfExtends.y;
        float maxZ = boxCenter.z + collider->HalfExtends.z;

        float tmin = -2147483647.0f;
        float tmax = 2147483647.0f;

        float txmin = (minX - ray.Origin.x) * ray.InvDirection.x;
        float txmax = (maxX - ray.Origin.x) * ray.InvDirection.x;

        if (txmin > txmax) std::swap(txmin, txmax);

        if ((tmin > txmin) || (txmin > tmax))
            return -1.f;

        if (txmin > tmin) tmin = txmin;
        if (txmax < tmax) tmax = txmax;

        float tymin = (minY - ray.Origin.y) * ray.InvDirection.y;
        float tymax = (maxY - ray.Origin.y) * ray.InvDirection.y;

        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax))
            return -1.f;

        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (minZ - ray.Origin.z) * ray.InvDirection.z;
        float tzmax = (maxZ - ray.Origin.z) * ray.InvDirection.z;

        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax))
            return -1.f;

        if (tzmin > tmin) tmin = tzmin;
        if (tzmax < tmax) tmax = tzmax;

        float tNear = glm::max(glm::max(txmin, tymin), tzmin);

        if (tNear == txmin)
            hit.ContactNormal = glm::vec3((ray.InvDirection.x < 0) ? 1 : -1, 0, 0);
        else if (tNear == tymin)
            hit.ContactNormal = glm::vec3(0, (ray.InvDirection.y < 0) ? 1 : -1, 0);
        else
            hit.ContactNormal = glm::vec3(0, 0, (ray.InvDirection.z < 0) ? 1 : -1);

        hit.ContactPoint = ray.Origin + ray.Direction * tmin;
        hit.HitEntity = collider->GetEntityID();

        return tmin;
    };

    bool Collision3DSystem::Raycast3D(RayHit& hit, const Ray& ray)
    {
        RayHit tempHit;
        float minDist = ray.maxDistance;
        std::vector<TraversedCells> traversedCells = RaycastGridDDA(ray);

        for (auto& cellData : traversedCells)
        {
            Cell& currentCell = Grid[cellData.CellCoords.x][cellData.CellCoords.y];
            if (currentCell.CellEntity.empty())
                continue;

            for (auto entity : currentCell.CellEntity)
            {
                if ( m_Ctx.pScene->GetComponent<Collision3DComponent>(entity)->IsSphere)
                {
                    float dist = RaySphereCollision(
                        tempHit,
                        ray,
                        m_Ctx.pScene->GetComponent<Collision3DComponent>(entity),
                        m_Ctx.pScene->GetComponent<Transform3D>(entity)
                        );
                    if (dist < 0) continue;

                    if (dist < minDist)
                    {
                        minDist = dist;
                        hit = tempHit;
                    }
                }
                else
                {
                    float dist = RayAABBCollision(tempHit, ray,
                        m_Ctx.pScene->GetComponent<Collision3DComponent>(entity),
                        m_Ctx.pScene->GetComponent<Transform3D>(entity));

                    if (dist < 0) continue;

                    if (dist < minDist)
                    {
                        minDist = dist;
                        hit = tempHit;
                    }
                }
            }

            if (minDist < cellData.minDistanceToNextCell)
                break;
        }

        return minDist < ray.maxDistance;
    }

    std::vector<TraversedCells> Collision3DSystem::RaycastGridDDA(const Ray& ray)
    {
        // reference : https://joelschumacher.de/posts/ray-casting-in-2d-grids AND https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
        std::vector<TraversedCells> cells;
        float x = (ray.Origin.x - GridOffset.x) / CellSize.x + GridSize.x * 0.5f;
        float z = (ray.Origin.z - GridOffset.y) / CellSize.y + GridSize.y * 0.5f;

        int mapX = static_cast<int>(std::floor(x));
        int mapZ = static_cast<int>(std::floor(z));

        glm::vec3 normal = glm::normalize(ray.Direction);
        float rayDirX = normal.x;
        float rayDirZ = normal.z;

        int stepX = (rayDirX < 0) ? -1 : 1;
        int stepZ = (rayDirZ < 0) ? -1 : 1;

        float deltaDistX = (rayDirX == 0) ? 1e30f : abs(1.0f / rayDirX);
        float deltaDistZ = (rayDirZ == 0) ? 1e30f : abs(1.0f / rayDirZ);

        float sideDistX;
        float sideDistZ;

        if (rayDirX < 0)
            sideDistX = (x - mapX) * deltaDistX;
        else
            sideDistX = (mapX + 1.0f - x) * deltaDistX;

        if (rayDirZ < 0)
            sideDistZ = (z - mapZ) * deltaDistZ;
        else
            sideDistZ = (mapZ + 1.0f - z) * deltaDistZ;

        float distance = 0.0f;

        if (mapX < 0 || mapX >= GridSize.x || mapZ < 0 || mapZ >= GridSize.y)
            return {};
        cells.push_back({{mapX, mapZ}, distance});

        while (distance < ray.maxDistance)
        {
            if (sideDistX < sideDistZ)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                distance = sideDistX;
            }
            else
            {
                sideDistZ += deltaDistZ;
                mapZ += stepZ;
                distance = sideDistZ;
            }

            if (mapX < 0 || mapX >= GridSize.x || mapZ < 0 || mapZ >= GridSize.y)
                break;

            cells.push_back({{mapX, mapZ}, distance});
        }

        return cells;
    }
}
