#include "Village/Map/Map.h"
#include "Scene.h"
#include "Render3DSystem.h"
#include "TransformComponents.h"
#include <cmath>

#include "Random.h"

namespace GPC
{

    GridPos GridPos::ToGridPos(glm::vec3 worldPos)
    {
        return { static_cast<int>(std::floor(worldPos.x)), static_cast<int>(std::floor(worldPos.z)) };
    }

    bool GridPos::operator<(const GridPos& other) const
    {
        if (X != other.X) return X < other.X;
        return Y < other.Y;
    }

    Map::Map(Scene* scene, glm::vec2 size) : mp_Scene(scene), m_Size(size)
    {
        m_MapEntity = mp_Scene->CreateEntityAs3D();
        mp_Render3DComponentGrid = mp_Scene->AddComponent<Render3DComponent>(m_MapEntity);
        mp_Render3DComponentGrid->pGeometry = GPC_MESH("Plane2x2");
        mp_Render3DComponentGrid->SetTexture("2067x2067", "Grid");
        mp_Render3DComponentGrid->IsEnable = false;

        mp_Transform = mp_Scene->GetComponent<Transform3D>(m_MapEntity);
        mp_Transform->LocalTransform.SetPosition(m_Size.x * 0.5f, 0.0f, m_Size.y * 0.5f);
        mp_Transform->LocalTransform.SetScale(glm::vec3(m_Size.x * 1.0f, 1.0f, m_Size.y * 1.0f));
        mp_Transform->LocalTransform.SetRotationYawPitchRoll(
            glm::radians(VillageGroundTilt::YAW_DEG),
            glm::radians(VillageGroundTilt::PITCH_DEG),
            glm::radians(VillageGroundTilt::ROLL_DEG));

        EntityID ground = mp_Scene->CreateEntityAs3D();
        mp_Render3DComponentGround = mp_Scene->AddComponent<Render3DComponent>(ground);
        mp_Render3DComponentGround->pGeometry = GPC_MESH("Plane");
        mp_Render3DComponentGround->SetTexture("2048x2048", "Ground");
        mp_Render3DComponentGround->UseShadow = true;

        auto transform = mp_Scene->GetComponent<Transform3D>(ground);
        transform->LocalTransform.SetPosition(m_Size.x * 0.5f, -0.02f, m_Size.y * 0.5f);
        transform->LocalTransform.SetScale(glm::vec3(m_Size.x * 1.0f, 1.0f, m_Size.y * 1.0f));
        transform->LocalTransform.SetRotationYawPitchRoll(
            glm::radians(VillageGroundTilt::YAW_DEG),
            glm::radians(VillageGroundTilt::PITCH_DEG),
            glm::radians(VillageGroundTilt::ROLL_DEG));

        mp_Scene->AddComponent<Collision3DComponent>(m_MapEntity);

        constexpr int   WALL_COUNT      = 5.0f;
        constexpr int   LAYER_COUNT     = 3.0f;

        const glm::vec3 mapCenter(m_Size.x * 0.5f, 0.0f, m_Size.y * 0.5f);
        const glm::quat tiltQuat = Transform::YawPitchRoll_To_Quaternion(
            glm::radians(VillageGroundTilt::YAW_DEG),
            glm::radians(VillageGroundTilt::PITCH_DEG),
            glm::radians(VillageGroundTilt::ROLL_DEG));

        {
            for (int y = 0; y < LAYER_COUNT; ++y) {
                for (int i = -1; i < WALL_COUNT; ++i) {
                    EntityID background = mp_Scene->CreateEntityAs3D();

                    const float WALL_SIZE_X     = (70.0f + y * 4.0f) / WALL_COUNT;
                    const float WALL_SIZE_Y     = WALL_SIZE_X * 1.4035087719f;

                    auto bTransform = mp_Scene->GetComponent<Transform3D>(background);

                    const glm::vec3 originalPos(70.0f + y * 4.0f, WALL_SIZE_Y / 2.0f + y * 2.0f,
                                                WALL_SIZE_X / 2.0f + WALL_SIZE_X * i);
                    const glm::vec3 rel = originalPos - mapCenter;
                    const glm::vec3 rotatedRel = tiltQuat * rel;
                    bTransform->LocalTransform.SetPosition(mapCenter + rotatedRel);

                    bTransform->LocalTransform.SetScale( {WALL_SIZE_X, 1, WALL_SIZE_Y} );
                    bTransform->LocalTransform.RotateYawPitchRoll(0.0f, -GPC_PI / 2.0f, 0.0f);
                    bTransform->LocalTransform.RotateYawPitchRoll(GPC_PI / 2.0f, 0.0f, GPC_PI);
                    bTransform->LocalTransform.RotateYawPitchRoll(
                        glm::radians(VillageGroundTilt::YAW_DEG),
                        glm::radians(VillageGroundTilt::PITCH_DEG),
                        glm::radians(VillageGroundTilt::ROLL_DEG));

                    Render3DComponent* bRender = mp_Scene->AddComponent<Render3DComponent>(background);
                    bRender->pGeometry = GPC_MESH("Plane");
                    if (i == -1) {
                        bTransform->LocalTransform.AddPosition({0.0, 0.0, 0.1});
                        bRender->SetTexture("1368x1920", "Village_Background_End_Flipped");
                    } else {
                        bRender->SetTexture("1368x1920", "Village_Background");
                    }
                    bRender->IsEnable = true;
                }
            }
        }

        {

            for (int y = 0; y < LAYER_COUNT; ++y) {
                for (int i = -1; i < WALL_COUNT; ++i) {
                    EntityID background = mp_Scene->CreateEntityAs3D();

                    const float WALL_SIZE_X     = (70.0f + y * 4.0f) / WALL_COUNT;
                    const float WALL_SIZE_Y     = WALL_SIZE_X * 1.4035087719f;

                    auto bTransform = mp_Scene->GetComponent<Transform3D>(background);

                    const glm::vec3 originalPos(WALL_SIZE_X / 2.0f + WALL_SIZE_X * i,
                                                WALL_SIZE_Y / 2.0f + y * 2.0f,
                                                70.0f + y * 4.0f);
                    const glm::vec3 rel = originalPos - mapCenter;
                    const glm::vec3 rotatedRel = tiltQuat * rel;
                    bTransform->LocalTransform.SetPosition(mapCenter + rotatedRel);

                    bTransform->LocalTransform.SetScale( {WALL_SIZE_X, 1, WALL_SIZE_Y} );
                    bTransform->LocalTransform.RotateYawPitchRoll(0.0f, -GPC_PI / 2.0f, 0.0f);
                    bTransform->LocalTransform.RotateYawPitchRoll(0.0f, 0.0f, GPC_PI);
                    bTransform->LocalTransform.RotateYawPitchRoll(
                        glm::radians(VillageGroundTilt::YAW_DEG),
                        glm::radians(VillageGroundTilt::PITCH_DEG),
                        glm::radians(VillageGroundTilt::ROLL_DEG));

                    Render3DComponent* bRender = mp_Scene->AddComponent<Render3DComponent>(background);
                    bRender->pGeometry = GPC_MESH("Plane");
                    if (i == -1) {
                        bTransform->LocalTransform.AddPosition({0.1, 0.0, 0.0});
                        bRender->SetTexture("1368x1920", "Village_Background_End");
                    } else {
                        bRender->SetTexture("1368x1920", "Village_Background");
                    }
                    bRender->IsEnable = true;
                }
            }
        }

        SpawnDecoration(Random::Vector2(2.8f, 3.3f), { 20, 5 }, "PropsVillage", "Bush0");
        SpawnDecoration(Random::Vector2(2.8f, 3.3f), { 62, 8 }, "PropsVillage", "Bush1");
        SpawnDecoration(Random::Vector2(3.2f, 3.9f), { 65, 6 }, "PropsVillage", "Bush8");
        SpawnDecoration(Random::Vector2(3.2f, 3.9f), { 32, 48 }, "PropsVillage", "Bush9");
        SpawnDecoration(Random::Vector2(2.8f, 3.3f), { 22, 35 }, "PropsVillage", "Bush10");
        SpawnDecoration(Random::Vector2(2.8f, 3.3f), { 8, 30 }, "PropsVillage", "Bush3");
        SpawnDecoration(Random::Vector2(2.8f, 3.3f), { 23, 25 }, "PropsVillage", "Bush4");
        SpawnDecoration(Random::Vector2(2.8f, 3.3f), { 38, 45 }, "PropsVillage", "Bush3");
        SpawnDecoration(Random::Vector2(3.2f, 3.9f), { 12, 50 }, "PropsVillage", "Bush10");
        SpawnDecoration(Random::Vector2(3.2f, 3.9f), { 38, 50 }, "PropsVillage", "Bush11");
        SpawnDecoration(Random::Vector2(3.2f, 3.9f), { 62, 48 }, "PropsVillage", "Bush12");

        SpawnDecoration(Random::Vector2(2.8f, 3.4f), { 15, 15 }, "ResourceAndDecor", "Cailloux0");
        SpawnDecoration(Random::Vector2(2.8f, 3.4f), { 38, 7 }, "ResourceAndDecor", "Cailloux1");
        SpawnDecoration(Random::Vector2(2.8f, 3.4f), { 46, 22 }, "ResourceAndDecor", "Cailloux2");
        SpawnDecoration(Random::Vector2(2.8f, 3.4f), { 5, 39 }, "ResourceAndDecor", "Cailloux3");
        SpawnDecoration(Random::Vector2(2.8f, 3.4f), { 28, 48 }, "ResourceAndDecor", "Cailloux4");
        SpawnDecoration(Random::Vector2(2.8f, 3.4f), { 58, 57 }, "ResourceAndDecor", "Cailloux5");

        SpawnTree(Random::Vector2(2.8f, 5.4f), { 30, 12 },  "Tronc0", "Leaves0_0");
        SpawnTree(Random::Vector2(2.8f, 5.4f), { 45, 5 },   "Tronc1", "Leaves3_1");
        SpawnTree(Random::Vector2(2.8f, 5.4f), { 60, 18 },  "Tronc2", "Leaves1_2");
        SpawnTree(Random::Vector2(2.8f, 5.4f), { 52, 60 },  "Tronc3", "Leaves5_3");
        SpawnTree(Random::Vector2(2.8f, 5.4f), { 11, 32 },  "Tronc1", "Leaves3_1");
        SpawnTree(Random::Vector2(2.8f, 5.4f), { 21, 63 },  "Tronc2", "Leaves2_2");
        SpawnTree(Random::Vector2(2.8f, 5.4f), { 47, 55 },  "Tronc3", "Leaves4_3");
        SpawnTree(Random::Vector2(2.8f, 5.4f), { 40, 35 },  "Tronc2", "Leaves4_2");

    }

    Map::~Map()
    {
        if (mp_Scene && m_MapEntity != 0)
        {
            mp_Scene->DestroyEntity(m_MapEntity);
        }
    }

    glm::vec2 Map::GetSize() const
    {
        return m_Size;
    }

    float Map::GetGroundYAt(float worldX, float worldZ) const
    {
        const glm::quat tiltQuat = Transform::YawPitchRoll_To_Quaternion(
            glm::radians(VillageGroundTilt::YAW_DEG),
            glm::radians(VillageGroundTilt::PITCH_DEG),
            glm::radians(VillageGroundTilt::ROLL_DEG));
        const glm::vec3 normal = tiltQuat * glm::vec3(0.0f, 1.0f, 0.0f);
        if (std::abs(normal.y) < 1e-6f) return 0.0f;
        const float dx = worldX - m_Size.x * 0.5f;
        const float dz = worldZ - m_Size.y * 0.5f;
        return -(normal.x * dx + normal.z * dz) / normal.y;
    }

    glm::vec2 Map::ProjectRayToTiltedGround(glm::vec3 rayOrigin, glm::vec3 rayDir) const
    {
        const glm::quat tiltQuat = Transform::YawPitchRoll_To_Quaternion(
            glm::radians(VillageGroundTilt::YAW_DEG),
            glm::radians(VillageGroundTilt::PITCH_DEG),
            glm::radians(VillageGroundTilt::ROLL_DEG));
        const glm::vec3 normal = tiltQuat * glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 planeCenter(m_Size.x * 0.5f, 0.0f, m_Size.y * 0.5f);

        const float denom = glm::dot(normal, rayDir);
        if (std::abs(denom) < 1e-6f) return glm::vec2(rayOrigin.x, rayOrigin.z);

        const float t = glm::dot(normal, planeCenter - rayOrigin) / denom;
        if (t < 0.0f) return glm::vec2(rayOrigin.x, rayOrigin.z);

        const glm::vec3 hit = rayOrigin + t * rayDir;
        return glm::vec2(hit.x, hit.z);
    }

    bool Map::IsSpaceFree(glm::ivec2 position, glm::ivec2 size) const
    {
        for (int x = 0; x < size.x; ++x)
        {
            for (int y = 0; y < size.y; ++y)
            {
                GridPos currentPos = { position.x + x, position.y + y };

                if (IsOutOfBounds(currentPos))
                    return false;

                if (IsCellOccupied(currentPos))
                    return false;
            }
        }
        return true;
    }

    void Map::RegisterBuilding(glm::ivec2 position, glm::ivec2 size, EntityID id)
    {
        Transform3D* transform;
        Render3DComponent* renderer;
        const float wx = position.x + size.x * 0.5f;
        const float wz = position.y + size.y * 0.5f;
        EntityID grassId = mp_Scene->CreateModel(
            {wx, GetGroundYAt(wx, wz) + 0.02f, wz},
            "Red",
            "Plane4x4",
            &transform,
            &renderer
        );
        transform->LocalTransform.SetScale({(float)size.x, 1.0f, (float)size.y});
        transform->LocalTransform.SetRotationYawPitchRoll(
            glm::radians(VillageGroundTilt::YAW_DEG),
            glm::radians(VillageGroundTilt::PITCH_DEG),
            glm::radians(VillageGroundTilt::ROLL_DEG));
        renderer->IsEnable = false;
        // renderer->RenderMaterial.Tint = Colors::DARK_RED;
        m_UsedTiles.push_back(renderer);

        m_BuildingToGrass[id] = grassId;

        for (int x = 0; x < size.x; ++x)
        {
            for (int y = 0; y < size.y; ++y)
            {
                GridPos occupyPos = { position.x + x, position.y + y };
                m_Buildings[occupyPos] = id;
                if (m_Decorations.contains(occupyPos))
                    m_Decorations[occupyPos]->IsEnable = false;
            }
        }
    }

    void Map::RemovePlacedZone(EntityID id)
    {
        auto itGrass = m_BuildingToGrass.find(id);
        if (itGrass != m_BuildingToGrass.end())
        {
            auto grass = itGrass->second;
            mp_Scene->GetComponent<Transform3D>(grass)->LocalTransform.SetScale(0, 0, 0);
        }
        if (auto* render = mp_Scene->GetComponent<Render3DComponent>(id))
            render->IsEnable = false;

        for (auto it = m_Buildings.begin(); it != m_Buildings.end(); )
        {
            if (it->second == id)
                it = m_Buildings.erase(it);
            else
                ++it;
        }

    }

    EntityID Map::GetEntityAt(glm::ivec2 position) const
    {
        GridPos pos = { position.x, position.y };
        auto it = m_Buildings.find(pos);
        if (it != m_Buildings.end())
        {
            return it->second;
        }
        return 0;
    }

    void Map::Clear()
    {
        m_BuildingToGrass.clear();
        m_Buildings.clear();
    }

    void Map::Update()
    {
    }

    void Map::SetGridActive(bool active)
    {
        mp_Render3DComponentGrid->IsEnable = active;
        for (auto usedTile : m_UsedTiles) {
            usedTile->IsEnable = active;
        }
    }

    void Map::SpawnDecoration(glm::vec2 size, glm::ivec2 position, std::string array, std::string texture, float width, float height)
    {
        EntityID entity = mp_Scene->CreateEntityAs3D();
        Transform3D* transformSprite = mp_Scene->GetComponent<Transform3D>(entity);

        const float texW    = width;
        const float texH    = height;
        const float perspectiveRatio = 1.4f;
        const float scale   = 1.0f;

        float spriteW = (float)size.x;
        float spriteH = spriteW * (texH / texW) * perspectiveRatio;
        transformSprite->LocalTransform.SetScale({ spriteW * scale, 1.0f, spriteH * scale });

        // Position + rotation d'abord. Y projetee sur le sol tilte pour visuel coherent.
        const float spriteX = position.x;
        const float spriteZ = position.y;
        const float spriteY = GetGroundYAt(spriteX, spriteZ) + 1.0f;
        transformSprite->LocalTransform.SetPosition(glm::vec3(spriteX, spriteY, spriteZ));
        transformSprite->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.5f, GPC_PI);

        // size.x * 0.5f sur right, size.y * 0.5f sur forward pour centrer sur le cube
        transformSprite->LocalTransform.AddPosition(-transformSprite->LocalTransform.GetForward() * size.y * scale * VillageGroundTilt::SPRITE_DEPTH_FACTOR);
        transformSprite->LocalTransform.AddPosition(transformSprite->LocalTransform.GetRight() * size.x * scale * VillageGroundTilt::SPRITE_LATERAL_FACTOR);

        Render3DComponent* renderer = mp_Scene->AddComponent<Render3DComponent>(entity);
        renderer->pGeometry = GPC_MESH("PlaneDS");
        renderer->UseShadow = true;
        renderer->SetTexture(array, texture);

        GridPos pos = { position.x, position.y };
        m_Decorations[pos] = renderer;
    }

    void Map::SpawnTree(glm::vec2 size, glm::ivec2 position, std::string tronc, std::string feuille)
    {
        SpawnDecoration(size, position, "ResourceAndDecor", feuille, 1024, 1024);
        SpawnDecoration(size, position, "ResourceAndDecor", tronc, 1024, 1024);
    }

    bool Map::IsOutOfBounds(const GridPos& pos) const
    {
        return pos.X < 0 || pos.X >= static_cast<int>(m_Size.x) ||
               pos.Y < 0 || pos.Y >= static_cast<int>(m_Size.y);
    }

    bool Map::IsCellOccupied(const GridPos& pos) const
    {
        return m_Buildings.count(pos) > 0;
    }
}
