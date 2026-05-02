#include "Map Generation/MapGenerator.h"

#include "Assets.h"
#include "Loader.h"
#include "Random.h"

namespace GPC {

    GENERATION_ID MapGeneratorFactory::CreateNew() {
        auto data = new MapGeneratorData();
        data->ID = m_Datas.size();
        m_Datas.push_back(data);
        return data->ID;
    }

    MapGeneratorData * MapGeneratorFactory::GetGeneration(GENERATION_ID id) {
        return m_Datas[id];
    }

    void MapGeneratorFactory::DestroyAll() {
        for (auto pData  : m_Datas) {
            pData->Destroy();
            delete pData;
        }
        for (auto pTileGroup : mp_TileGroup_GeometryDatas_CornerEmpty) {
            delete pTileGroup;
        }
        for (auto pTileGroup : mp_TileGroup_GeometryDatas_CornerFull) {
            delete pTileGroup;
        }
        for (auto pTileGroup : mp_TileGroup_GeometryDatas_Diag) {
            delete pTileGroup;
        }
        for (auto pTileGroup : mp_TileGroup_GeometryDatas_Full) {
            delete pTileGroup;
        }
        for (auto pTileGroup : mp_TileGroup_GeometryDatas_Line) {
            delete pTileGroup;
        }

        m_Created = false;
    }


    glm::ivec2 WorldPositionToRoomPosition(glm::vec3 world) {
        return glm::ivec2{
            world.x / MapGeneratorData::ROOM_SCALE,
            world.z / MapGeneratorData::ROOM_SCALE
        };
    }

    void MapGeneratorData::GenerateWithRoom(const MapMask &mask, const std::vector<RoomMeta> &metas) {
        std::vector<Room> rooms{metas.size()};
        for (uint32_t i = 0; i < metas.size(); ++i) {
            rooms[i] = GPC::Room::CreateRoomFromRaw(metas[i].GetRaw());
        }

        GeneratedMapMask = mask;
        GeneratedLayout.Resolve(GeneratedMapMask);
        GeneratedMissionData.ResolveFor(GeneratedMapMask, GeneratedLayout, rooms.data(), metas.data(), rooms.size());
        GeneratedTileGroupLayout = LayoutToGroupLayout(GeneratedMissionData);

        GenerateMapBorders();

        GenerateRoomDatas();

        GenerateMiniMapTexture();
    }

    void MapGeneratorData::Generate1Room(const RawRoom &raw) {
        GeneratedTileGroupLayout[0][0] = RawRoomToGroupLayout(raw);
        AddRoomData(0, 0);
    }

    void MapGeneratorData::Destroy() const {
        for (auto& data : GeneratedRoomDatas) {
            delete data.pGeometry;
        }
    }

    std::string MapGeneratorData::GetArrayName() const {
        return "Mini-Map-Array-" + std::to_string(ID);
    }

    std::string MapGeneratorData::GetTextureName() const {
        return "Mini-Map-Texture-" + std::to_string(ID);
    }

    void MapGeneratorData::GenerateMiniMapTexture() {
        constexpr uint64_t WIDTH = MAX_MAP_SIZE * TILE_GROUP_COUNT * 2;
        constexpr uint64_t HEIGHT = MAX_MAP_SIZE * TILE_GROUP_COUNT * 2;

        constexpr uint64_t BlobSize = WIDTH * HEIGHT * 4;
        uint8_t pBlob[BlobSize];

        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {

                for (uint32_t x_tile = 0; x_tile < TILE_GROUP_COUNT; ++x_tile) {
                    for (uint32_t y_tile = 0; y_tile < TILE_GROUP_COUNT; ++y_tile) {

                        for (uint32_t tx = 0; tx < 2; ++tx) {
                            for (uint32_t ty = 0; ty < 2; ++ty) {

                                uint32_t x = x_room * TILE_GROUP_COUNT * 2 + x_tile * 2 + tx;
                                uint32_t y = y_room * TILE_GROUP_COUNT * 2 + y_tile * 2 + ty;

                                uint8_t* pPixel = pBlob + (WIDTH * (HEIGHT - y - 1) + (WIDTH - x - 1)) * RGBA;
                                if (GeneratedTileGroupLayout[x_room][y_room][x_tile][y_tile].b[tx][ty]) {
                                    if (GeneratedLayout.GetEnd().x == x_room && GeneratedLayout.GetEnd().y == y_room) {
                                        pPixel[0] = 0;
                                        pPixel[1] = 255;
                                        pPixel[2] = 0;
                                        pPixel[3] = 255;
                                    }
                                    else {
                                        pPixel[0] = 255;
                                        pPixel[1] = 255;
                                        pPixel[2] = 255;
                                        pPixel[3] = 255;
                                    }
                                }
                                else {
                                    pPixel[0] = 0;
                                    pPixel[1] = 0;
                                    pPixel[2] = 0;
                                    pPixel[3] = 0;
                                }
                            }
                        }


                    }
                }

            }
        }

        auto tex = GPC_ASSETS->AddTexture(
            GetTextureName(),
            pBlob, WIDTH, HEIGHT
        );

        GPC_ASSETS->AddTextureArray(
            GetArrayName(),
            WIDTH, HEIGHT,
            {
                tex
            }
        );

        // delete[] pBlob;
    }

    void MapGeneratorData::GenerateMapBorders() {
        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                SolveMapBorderForRoom(x_room, y_room);
            }
        }
    }

    void MapGeneratorData::SolveMapBorderForRoom(uint32_t x_room, uint32_t y_room) {
        constexpr uint32_t SUB_DIVISION = 2;
        constexpr uint32_t TILE_COUNT = TILE_GROUP_COUNT * SUB_DIVISION;

        bool visited[TILE_COUNT][TILE_COUNT] = {};
        for (uint32_t y = 0; y < TILE_COUNT; ++y) {
            for (uint32_t x = 0; x < TILE_COUNT; ++x) {
                visited[x][y] = false;
            }
        }

        for (uint32_t x = 0; x < TILE_COUNT; ++x) {
            for (uint32_t y = 0; y < TILE_COUNT; ++y) {
                uint32_t tile_x = x / SUB_DIVISION;
                uint32_t tile_y = y / SUB_DIVISION;
                uint32_t sub_tile_x = x % SUB_DIVISION;
                uint32_t sub_tile_y = y % SUB_DIVISION;
                auto v_tile = GeneratedTileGroupLayout[x_room][y_room][tile_x][tile_y].b[sub_tile_x][sub_tile_y];
                if (visited[x][y] || v_tile != EMPTY) continue;

                MapBorder border{};
                if (ExpandRectFrom(border, x_room, y_room, x, y, visited)) {
                    GeneratedBorders.push_back(border);
                }
            }
        }
    }

    bool MapGeneratorData::ExpandRectFrom(MapBorder &out, uint32_t x_room, uint32_t y_room, uint32_t x_tile, uint32_t y_tile, bool(*visited)[34]) const {
        constexpr uint32_t SUB_DIVISION = 2;
        constexpr uint32_t TILE_COUNT = TILE_GROUP_COUNT * SUB_DIVISION;

        if (visited[x_tile][y_tile]) return false;

        uint32_t w = 0;
        for (uint32_t x = x_tile; x < TILE_COUNT; ++x) {
            uint32_t tile_x = x / SUB_DIVISION;
            uint32_t tile_y = y_tile / SUB_DIVISION;
            uint32_t sub_tile_x = x % SUB_DIVISION;
            uint32_t sub_tile_y = y_tile % SUB_DIVISION;
            auto v_tile = GeneratedTileGroupLayout[x_room][y_room][tile_x][tile_y].b[sub_tile_x][sub_tile_y];
            if (visited[x][y_tile] || v_tile != EMPTY) {
                break;
            }
            w++;
        }

        uint32_t lastColumn = x_tile + w;
        uint32_t h = 0;
        for (uint32_t y = y_tile; y < TILE_COUNT; ++y) {
            for (uint32_t x = x_tile; x < lastColumn; ++x) {
                uint32_t tile_x = x / 2;
                uint32_t tile_y = y / 2;
                uint32_t sub_tile_x = x % 2;
                uint32_t sub_tile_y = y % 2;
                auto v_tile = GeneratedTileGroupLayout[x_room][y_room][tile_x][tile_y].b[sub_tile_x][sub_tile_y];
                if (visited[x][y] || v_tile != EMPTY) {
                    goto END;
                }
            }
            h++;
        }
        END:

        uint32_t lastRow = y_tile + h;
        for (uint32_t y = y_tile; y < lastRow; ++y) {
            for (uint32_t x = x_tile; x < lastColumn; ++x) {
                visited[x][y] = true;
            }
        }

        if (w == 0 || h == 0) return false;

        out.Corner.x = x_room * TILE_COUNT + x_tile;
        out.Corner.y = y_room * TILE_COUNT + y_tile;
        out.Size.x = w;
        out.Size.y = h;
        return true;
    }

    void MapGeneratorData::GenerateRoomDatas() {
        for (uint32_t x_room = 0; x_room < MAX_MAP_SIZE; ++x_room) {
            for (uint32_t y_room = 0; y_room < MAX_MAP_SIZE; ++y_room) {
                AddRoomData(x_room, y_room);
            }
        }
    }

    void MapGeneratorData::AddRoomData(uint32_t x_room, uint32_t y_room) {
        GeometryData CurrentGeometryData{};
        uint32_t CurrentGeometryDataVerticesIndex = 0;

        for (uint32_t x_tile_group = 0; x_tile_group < TILE_GROUP_COUNT; ++x_tile_group) {
            for (uint32_t y_tile_group = 0; y_tile_group < TILE_GROUP_COUNT; ++y_tile_group) {
                auto tile_grp = GeneratedTileGroupLayout[x_room][y_room][x_tile_group][y_tile_group];

                auto& solutions = MapGeneratorFactory::m_Resolver[tile_grp];
                if (solutions.empty()) continue;

                TileGroupSolution& solution = solutions[rand() % solutions.size()];
                AddTileGroupToGeometryData(CurrentGeometryData, CurrentGeometryDataVerticesIndex, solution, x_tile_group, y_tile_group);
            }
        }

        if (CurrentGeometryDataVerticesIndex == 0) return;

        Geometry* pGeometry = new Geometry();
        pGeometry->Create(&CurrentGeometryData);
        GeneratedRoomDatas.push_back({
            .pGeometry = pGeometry,
            .pMeta = GeneratedMissionData.GetMeta(x_room, y_room),
            .Rotation =  GeneratedMissionData.GetRotation(x_room, y_room),
            .Position = glm::vec3{ ROOM_SCALE * x_room, 0, ROOM_SCALE * y_room },
            .Scale = glm::vec3{TILE_GROUP_SCALE, TILE_GROUP_SCALE, TILE_GROUP_SCALE},
            .MapPosition = glm::ivec2{x_room, y_room},
        });
    }

    void MapGeneratorData::AddTileGroupToGeometryData(GeometryData &out, uint32_t& vIndexStart, const TileGroupSolution &solution, uint32_t tile_group_x, uint32_t tile_group_y) {
        float _cos = glm::cos(glm::radians(solution.AngleY));
        float _sin = glm::sin(glm::radians(solution.AngleY));

        for (uint32_t vIndex = 0; vIndex < solution.pGeometry->Vertices.size(); ++vIndex) {
            glm::vec3 final_position = glm::vec3(
                (solution.pGeometry->Vertices[vIndex].Position.x * _cos - solution.pGeometry->Vertices[vIndex].Position.z * _sin) * SIZE_REDUCTION,
                solution.pGeometry->Vertices[vIndex].Position.y * SIZE_REDUCTION,
                (solution.pGeometry->Vertices[vIndex].Position.x * _sin + solution.pGeometry->Vertices[vIndex].Position.z * _cos) * SIZE_REDUCTION
            );
            glm::vec3 final_normal = glm::vec3(
                solution.pGeometry->Vertices[vIndex].Normals.x * _cos - solution.pGeometry->Vertices[vIndex].Normals.z * _sin,
                solution.pGeometry->Vertices[vIndex].Normals.y,
                solution.pGeometry->Vertices[vIndex].Normals.x * _sin + solution.pGeometry->Vertices[vIndex].Normals.z * _cos
            );
            glm::vec2 final_texCoord = glm::vec2(
                solution.pGeometry->Vertices[vIndex].TextureCoordinate.x * _cos - solution.pGeometry->Vertices[vIndex].TextureCoordinate.y * _sin,
                solution.pGeometry->Vertices[vIndex].TextureCoordinate.x * _sin + solution.pGeometry->Vertices[vIndex].TextureCoordinate.y * _cos
            );

            final_position += glm::vec3{ static_cast<float>(tile_group_x) + 0.5f, 0, static_cast<float>(tile_group_y) + 0.5f };
            final_normal = glm::normalize(final_normal);
            final_texCoord += glm::vec2(tile_group_x, tile_group_y);

            out.Vertices.push_back({
                final_position.x, final_position.y, final_position.z,
                final_normal.x, final_normal.y, final_normal.z,
                final_texCoord.x, final_texCoord.y
            });
        }

        for (uint32_t iIndex = 0; iIndex < solution.pGeometry->Indices.size(); ++iIndex) {
            out.Indices.push_back(vIndexStart + solution.pGeometry->Indices[iIndex]);
        }
        vIndexStart += solution.pGeometry->Vertices.size();

    }


    bool MapGeneratorFactory::LoadGeometryData(std::vector<GPC::GeometryData *> &out, const std::string &resource_name, const std::string &file_path) {
        if (GPC_FBX->Load(resource_name, file_path) == nullptr) return false;
        GPC_INFO << "LOADED FBX : " << file_path << ENDL;
        auto* mesh_asset = GPC_ASSETS->GetAssets(resource_name)->ToMesh();
        if (mesh_asset == nullptr) return false;
        out.push_back(mesh_asset->pGeometryData);
        return true;
    }

    void MapGeneratorFactory::GenerateResolver() {

        m_Resolver[{
            false, false,
            false, false
        }] = {};

        TileGroup tile_grp = {
            true, true,
            true, true
        };
        for (int i = 0; i < 4; ++i) {
            for (auto pData : mp_TileGroup_GeometryDatas_Full) {
                m_Resolver[tile_grp].push_back({ pData, i * 90.0f });
            }
            tile_grp = tile_grp.Rotated90();
        }

        tile_grp = {
            true, false,
            true, true
        };
        for (int i = 0; i < 4; ++i) {
            for (auto pData : mp_TileGroup_GeometryDatas_CornerEmpty) {
                m_Resolver[tile_grp].push_back({ pData, i * 90.0f });
            }
            tile_grp = tile_grp.Rotated90();
        }

        tile_grp = {
            false, false,
            true, false
        };
        for (int i = 0; i < 4; ++i) {
            for (auto pData : mp_TileGroup_GeometryDatas_CornerFull) {
                m_Resolver[tile_grp].push_back({ pData, i * 90.0f });
            }
            tile_grp = tile_grp.Rotated90();
        }

        tile_grp = {
            true, false,
            true, false
        };
        for (int i = 0; i < 4; ++i) {
            for (auto pData : mp_TileGroup_GeometryDatas_Line) {
                m_Resolver[tile_grp].push_back({ pData, i * 90.0f });
            }
            tile_grp = tile_grp.Rotated90();
        }

        tile_grp = {
            true, false,
            false, true
        };
        for (int i = 0; i < 4; ++i) {
            for (auto pData : mp_TileGroup_GeometryDatas_Diag) {
                m_Resolver[tile_grp].push_back({ pData, i * 90.0f });
            }
            tile_grp = tile_grp.Rotated90();
        }

    }

    void MapGeneratorFactory::CreateFactory() {
        if (m_Created) return;

        GPC::FbxLoader loader;

        uint32_t i = 1;
        do {
            if (LoadGeometryData(
                mp_TileGroup_GeometryDatas_Full,
                std::string("Full_") + std::to_string(i),
                "Resources/Models/MapTileGroup/full_" + std::to_string(i) + ".fbx"
                ) == false) break;
            i++;
        } while (true);

        i = 1;
        do {
            if (LoadGeometryData(
                mp_TileGroup_GeometryDatas_Line,
                std::string("Line_") + std::to_string(i),
                "Resources/Models/MapTileGroup/line_" + std::to_string(i) + ".fbx"
                ) == false) break;
            i++;
        } while (true);

        i = 1;
        do {
            if (LoadGeometryData(
                mp_TileGroup_GeometryDatas_CornerFull,
                std::string("CornerFull_") + std::to_string(i),
                "Resources/Models/MapTileGroup/cornerFull_" + std::to_string(i) + ".fbx"
                ) == false) break;
            i++;
        } while (true);

        i = 1;
        do {
            if (LoadGeometryData(
                mp_TileGroup_GeometryDatas_CornerEmpty,
                std::string("CornerEmpty_") + std::to_string(i),
                "Resources/Models/MapTileGroup/cornerEmpty_" + std::to_string(i) + ".fbx"
                ) == false) break;
            i++;
        } while (true);

        i = 1;
        do {
            if (LoadGeometryData(
                mp_TileGroup_GeometryDatas_Diag,
                std::string("Diag_") + std::to_string(i),
                "Resources/Models/MapTileGroup/diag_" + std::to_string(i) + ".fbx"
                ) == false) break;
            i++;
        } while (true);

        GenerateResolver();

        m_Created = true;
    }
} // GPC