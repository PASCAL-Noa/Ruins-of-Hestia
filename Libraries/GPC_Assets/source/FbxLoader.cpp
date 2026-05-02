#include "FbxLoader.h"

std::string DataViewToString(ofbx::DataView view)
{
    if (!view.begin || !view.end || view.begin >= view.end)
        return {};

    return std::string(reinterpret_cast<const char*>(view.begin), reinterpret_cast<const char*>(view.end));
}

GPC::FbxAnimationProperty GetAnimationPropertyFromName(const std::string& propertyName)
{
    if (propertyName.find("Translation") != std::string::npos)
        return GPC::FbxAnimationProperty::Translation;
    if (propertyName.find("Rotation") != std::string::npos)
        return GPC::FbxAnimationProperty::Rotation;
    if (propertyName.find("Scaling") != std::string::npos)
        return GPC::FbxAnimationProperty::Scaling;

    return GPC::FbxAnimationProperty::Unknown;
}

void LoadFbxAnimations(ofbx::IScene* scene, GPC::FbxMeshData* rootData)
{
    if (!scene || !rootData)
        return;

    auto* animationData = new GPC::FbxAnimationData();
    const int animationStackCount = scene->getAnimationStackCount();
    animationData->Stacks.reserve(animationStackCount > 0 ? animationStackCount : 0);

    for (int stackIndex = 0; stackIndex < animationStackCount; ++stackIndex)
    {
        const ofbx::AnimationStack* stack = scene->getAnimationStack(stackIndex);
        if (!stack)
            break;

        GPC::FbxAnimationStackData stackData;
        stackData.Name = stack->name;
        if (stackData.Name.empty())
            stackData.Name = "AnimationStack" + std::to_string(stackIndex);
        bool hasTimeRange = false;

        const ofbx::TakeInfo* takeInfo = scene->getTakeInfo(stackData.Name.c_str());
        if (takeInfo)
        {
            stackData.StartTime = ofbx::fbxTimeToSeconds(takeInfo->local_time_from);
            stackData.EndTime = ofbx::fbxTimeToSeconds(takeInfo->local_time_to);
            hasTimeRange = true;
        }

        for (int layerIndex = 0;; ++layerIndex)
        {
            const ofbx::AnimationLayer* layer = stack->getLayer(layerIndex);
            if (!layer)
                break;

            GPC::FbxAnimationLayerData layerData;
            layerData.Name = layer->name;
            if (layerData.Name.empty())
                layerData.Name = "AnimationLayer" + std::to_string(layerIndex);

            for (int nodeIndex = 0;; ++nodeIndex)
            {
                const ofbx::AnimationCurveNode* curveNode = layer->getCurveNode(nodeIndex);
                if (!curveNode)
                    break;

                GPC::FbxAnimationNodeData nodeData;
                const ofbx::Object* targetBone = curveNode->getBone();
                if (targetBone) {
                    nodeData.TargetName = targetBone->name;
                } else if (curveNode->name[0] != '\0') {
                    nodeData.TargetName = std::string(curveNode->name);
                } else {
                    nodeData.TargetName.clear();
                }
                const std::string propertyName = DataViewToString(curveNode->getBoneLinkProperty());
                nodeData.Property = GetAnimationPropertyFromName(propertyName);

                for (int channelIndex = 0; channelIndex < 3; ++channelIndex)
                {
                    const ofbx::AnimationCurve* curve = curveNode->getCurve(channelIndex);
                    auto& curveData = nodeData.Curves[static_cast<size_t>(channelIndex)];
                    if (!curve)
                        continue;

                    const int keyCount = curve->getKeyCount();
                    if (keyCount <= 0)
                        continue;

                    const ofbx::i64* keyTimes = curve->getKeyTime();
                    const float* keyValues = curve->getKeyValue();
                    if (!keyTimes || !keyValues)
                        continue;

                    curveData.Keys.reserve(static_cast<size_t>(keyCount));
                    for (int keyIndex = 0; keyIndex < keyCount; ++keyIndex)
                    {
                        const double keyTimeSeconds = ofbx::fbxTimeToSeconds(keyTimes[keyIndex]);
                        const float keyValue = keyValues[keyIndex];
                        curveData.Keys.push_back({keyTimeSeconds, keyValue});

                        if (!hasTimeRange)
                        {
                            stackData.StartTime = keyTimeSeconds;
                            stackData.EndTime = keyTimeSeconds;
                            hasTimeRange = true;
                        }
                        else
                        {
                            stackData.StartTime = std::min(stackData.StartTime, keyTimeSeconds);
                            stackData.EndTime = std::max(stackData.EndTime, keyTimeSeconds);
                        }
                    }
                }

                layerData.Nodes.push_back(std::move(nodeData));
            }

            stackData.Layers.push_back(std::move(layerData));
        }

        animationData->Stacks.push_back(std::move(stackData));
    }

    rootData->pAnimationData = animationData;
}

void AddBoneInfluence(GPC::FbxVertexWeights& vertexWeights, uint16_t boneIndex, float weight)
{
    if (weight <= 0.0f)
        return;

    int emptySlot = -1;
    int minWeightSlot = 0;

    for (int i = 0; i < 4; ++i)
    {
        if (vertexWeights.Weights[i] == 0.0f && emptySlot < 0)
            emptySlot = i;

        if (vertexWeights.Weights[i] < vertexWeights.Weights[minWeightSlot])
            minWeightSlot = i;
    }

    int slot = emptySlot;
    if (slot < 0)
    {
        if (weight <= vertexWeights.Weights[minWeightSlot])
            return;

        slot = minWeightSlot;
    }

    vertexWeights.BoneIndices[slot] = boneIndex;
    vertexWeights.Weights[slot] = weight;
}

void NormalizeBoneInfluences(GPC::FbxVertexWeights& vertexWeights)
{
    float totalWeight = 0.0f;
    for (float weight : vertexWeights.Weights)
        totalWeight += weight;

    if (totalWeight <= 0.0f)
    {
        vertexWeights.BoneIndices = {0, 0, 0, 0};
        vertexWeights.Weights = {0.0f, 0.0f, 0.0f, 0.0f};
        return;
    }

    for (float& weight : vertexWeights.Weights)
        weight /= totalWeight;
}

namespace GPC
{
    void InitializeFbxMeshData(FbxMeshData* data)
    {
        if (!data)
            return;

        data->MeshName.clear();
        data->NodeName.clear();
        data->TextureFileNamesByType.clear();
        data->pGeometryData = nullptr;

        data->Position = glm::vec3(0.0f);
        data->Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        data->Scale = glm::vec3(1.0f);
        data->Bones.clear();
        data->VertexWeights.clear();
        data->HasSkin = false;
        data->pAnimationData = nullptr;

        data->pNext = nullptr;
    }

    FbxMeshData* CreateEmptyFbxMeshData()
    {
        auto* data = new FbxMeshData();
        InitializeFbxMeshData(data);
        return data;
    }

    void ExtractFbxData(ofbx::IScene* scene, FbxMeshData* fbxData, const std::string& name)
    {
        InitializeFbxMeshData(fbxData);

        FbxMeshData* currentFbxData = fbxData;
        const int meshCount = scene->getMeshCount();

        /// Mesh and Geometry
        for (int i = 0; i < meshCount; ++i)
        {
            const ofbx::Mesh* mesh = scene->getMesh(i);
            const ofbx::GeometryData& geom = mesh->getGeometryData();

            auto* fbxModel = new GeometryData();

            const auto positions = geom.getPositions();
            const auto normals = geom.getNormals();
            const auto uvs = geom.getUVs();

            fbxModel->Vertices.reserve(positions.count);

            for (int v = 0; v < positions.count; ++v)
            {
                const ofbx::Vec3 pos = positions.get(v);
                const ofbx::Vec3 normal = (v < normals.count) ? normals.get(v) : ofbx::Vec3{0.f, 1.f, 0.f};
                ofbx::Vec2 uv = (v < uvs.count) ? uvs.get(v) : ofbx::Vec2{0.f, 0.f};
                uv.y = 1.0f - uv.y; // Flip along y axis

                fbxModel->Vertices.emplace_back(pos.x, pos.y, pos.z, normal.x, normal.y, normal.z, uv.x, uv.y);
            }

            for (int p = 0; p < geom.getPartitionCount(); ++p)
            {
                const ofbx::GeometryPartition partition = geom.getPartition(p);

                for (int poly = 0; poly < partition.polygon_count; ++poly)
                {
                    const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[poly];
                    if (polygon.vertex_count < 3)
                        continue;

                    std::vector<int> tmp(static_cast<size_t>(polygon.vertex_count));
                    std::vector<int> triIndices(static_cast<size_t>((polygon.vertex_count - 2) * 3));

                    const uint32_t triIndexCount = ofbx::triangulate(geom, polygon, triIndices.data(), tmp.data());
                    if (triIndexCount < 3 || (triIndexCount % 3) != 0)
                        continue;

                    for (uint32_t j = 0; j + 2 < triIndexCount; j += 3)
                    {
                        const int idx0 = triIndices[j + 0];
                        const int idx1 = triIndices[j + 1];
                        const int idx2 = triIndices[j + 2];

                        const bool valid0 = idx0 >= 0 && idx0 < static_cast<int>(fbxModel->Vertices.size()) && idx0 <= static_cast<int>(std::numeric_limits<uint16_t>::max());
                        const bool valid1 = idx1 >= 0 && idx1 < static_cast<int>(fbxModel->Vertices.size()) && idx1 <= static_cast<int>(std::numeric_limits<uint16_t>::max());
                        const bool valid2 = idx2 >= 0 && idx2 < static_cast<int>(fbxModel->Vertices.size()) && idx2 <= static_cast<int>(std::numeric_limits<uint16_t>::max());

                        if (!valid0 || !valid1 || !valid2) {
                            continue;
                        }

                        fbxModel->Indices.push_back(static_cast<uint16_t>(idx0));
                        fbxModel->Indices.push_back(static_cast<uint16_t>(idx1));
                        fbxModel->Indices.push_back(static_cast<uint16_t>(idx2));
                    }
                }
            }

            currentFbxData->MeshName = i == 0 ? name : (name + std::to_string(i));
            currentFbxData->NodeName = std::string(mesh->name);
            currentFbxData->pGeometryData = fbxModel;
            currentFbxData->TextureFileNamesByType.clear();
            currentFbxData->Bones.clear();
            currentFbxData->VertexWeights.clear();
            currentFbxData->HasSkin = false;

            /// Skinning
            const ofbx::Skin* skin = mesh->getSkin();
            if (!skin)
            {
                const ofbx::Geometry* geometry = mesh->getGeometry();
                if (geometry)
                    skin = geometry->getSkin();
            }

            if (skin && skin->getClusterCount() > 0 && !fbxModel->Vertices.empty())
            {
                const auto positionAttributes = geom.getPositions();
                std::unordered_map<int, std::vector<int>> controlPointToVertices;
                controlPointToVertices.reserve(fbxModel->Vertices.size());

                for (int vertexIndex = 0; vertexIndex < static_cast<int>(fbxModel->Vertices.size()); ++vertexIndex)
                {
                    const int controlPointIndex = positionAttributes.indices ? positionAttributes.indices[vertexIndex] : vertexIndex;
                    controlPointToVertices[controlPointIndex].push_back(vertexIndex);
                }

                currentFbxData->VertexWeights.resize(fbxModel->Vertices.size());
                currentFbxData->HasSkin = true;

                std::vector<const ofbx::Object*> boneObjects;
                const int clusterCount = skin->getClusterCount();
                boneObjects.reserve(clusterCount);
                currentFbxData->Bones.reserve(clusterCount);

                std::unordered_map<const ofbx::Object*, int> boneIndexByObject;
                boneIndexByObject.reserve(clusterCount);

                for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
                {
                    const ofbx::Cluster* cluster = skin->getCluster(clusterIndex);
                    if (!cluster)
                        continue;

                    const ofbx::Object* boneObject = cluster->getLink();
                    if (!boneObject)
                        continue;

                    int boneIndex = -1;
                    const auto existingBoneIt = boneIndexByObject.find(boneObject);
                    if (existingBoneIt != boneIndexByObject.end())
                    {
                        boneIndex = existingBoneIt->second;
                    }
                    else
                    {
                        boneIndex = static_cast<int>(currentFbxData->Bones.size());
                        FbxBoneData boneData;
                        boneData.Name = boneObject->name;
                        boneData.RotationOrder = static_cast<int>(boneObject->getRotationOrder());
                        const ofbx::DVec3 rotationOffset = boneObject->getRotationOffset();
                        const ofbx::DVec3 rotationPivot = boneObject->getRotationPivot();
                        const ofbx::DVec3 preRotation = boneObject->getPreRotation();
                        const ofbx::DVec3 postRotation = boneObject->getPostRotation();
                        const ofbx::DVec3 scalingOffset = boneObject->getScalingOffset();
                        const ofbx::DVec3 scalingPivot = boneObject->getScalingPivot();
                        const ofbx::DVec3 localTranslation = boneObject->getLocalTranslation();
                        const ofbx::DVec3 localRotation = boneObject->getLocalRotation();
                        const ofbx::DVec3 localScaling = boneObject->getLocalScaling();
                        boneData.RotationOffset = glm::vec3(static_cast<float>(rotationOffset.x), static_cast<float>(rotationOffset.y), static_cast<float>(rotationOffset.z));
                        boneData.RotationPivot = glm::vec3(static_cast<float>(rotationPivot.x), static_cast<float>(rotationPivot.y), static_cast<float>(rotationPivot.z));
                        boneData.PreRotation = glm::vec3(static_cast<float>(preRotation.x), static_cast<float>(preRotation.y), static_cast<float>(preRotation.z));
                        boneData.PostRotation = glm::vec3(static_cast<float>(postRotation.x), static_cast<float>(postRotation.y), static_cast<float>(postRotation.z));
                        boneData.ScalingOffset = glm::vec3(static_cast<float>(scalingOffset.x), static_cast<float>(scalingOffset.y), static_cast<float>(scalingOffset.z));
                        boneData.ScalingPivot = glm::vec3(static_cast<float>(scalingPivot.x), static_cast<float>(scalingPivot.y), static_cast<float>(scalingPivot.z));
                        boneData.LocalTranslation = glm::vec3(static_cast<float>(localTranslation.x), static_cast<float>(localTranslation.y), static_cast<float>(localTranslation.z));
                        boneData.LocalRotation = glm::vec3(static_cast<float>(localRotation.x), static_cast<float>(localRotation.y), static_cast<float>(localRotation.z));
                        boneData.LocalScaling = glm::vec3(static_cast<float>(localScaling.x), static_cast<float>(localScaling.y), static_cast<float>(localScaling.z));
                        boneData.TransformMatrix = cluster->getTransformMatrix();
                        boneData.TransformLinkMatrix = cluster->getTransformLinkMatrix();
                        currentFbxData->Bones.push_back(boneData);
                        boneObjects.push_back(boneObject);
                        boneIndexByObject[boneObject] = boneIndex;
                    }

                    const int* clusterIndices = cluster->getIndices();
                    const double* clusterWeights = cluster->getWeights();
                    const int influenceCount = std::min(cluster->getIndicesCount(), cluster->getWeightsCount());

                    if (!clusterIndices || !clusterWeights || influenceCount <= 0)
                        continue;

                    for (int influenceIndex = 0; influenceIndex < influenceCount; ++influenceIndex)
                    {
                        const int controlPointIndex = clusterIndices[influenceIndex];
                        if (controlPointIndex < 0)
                            continue;

                        const auto verticesIt = controlPointToVertices.find(controlPointIndex);
                        const float weight = static_cast<float>(clusterWeights[influenceIndex]);
                        if (verticesIt != controlPointToVertices.end())
                        {
                            for (int vertexIndex : verticesIt->second)
                                AddBoneInfluence(currentFbxData->VertexWeights[vertexIndex], static_cast<uint16_t>(boneIndex), weight);
                            continue;
                        }

                        if (controlPointIndex < static_cast<int>(currentFbxData->VertexWeights.size()))
                            AddBoneInfluence(currentFbxData->VertexWeights[controlPointIndex], static_cast<uint16_t>(boneIndex), weight);
                    }
                }

                for (size_t boneIndex = 0; boneIndex < boneObjects.size(); ++boneIndex)
                {
                    const ofbx::Object* parent = boneObjects[boneIndex] ? boneObjects[boneIndex]->getParent() : nullptr;
                    while (parent)
                    {
                        if (boneIndexByObject.find(parent) != boneIndexByObject.end())
                        {
                            parent = parent->getParent();
                            continue;
                        }

                        FbxBoneData parentBoneData;
                        parentBoneData.Name = parent->name;
                        parentBoneData.RotationOrder = static_cast<int>(parent->getRotationOrder());
                        const ofbx::DVec3 rotationOffset = parent->getRotationOffset();
                        const ofbx::DVec3 rotationPivot = parent->getRotationPivot();
                        const ofbx::DVec3 preRotation = parent->getPreRotation();
                        const ofbx::DVec3 postRotation = parent->getPostRotation();
                        const ofbx::DVec3 scalingOffset = parent->getScalingOffset();
                        const ofbx::DVec3 scalingPivot = parent->getScalingPivot();
                        const ofbx::DVec3 localTranslation = parent->getLocalTranslation();
                        const ofbx::DVec3 localRotation = parent->getLocalRotation();
                        const ofbx::DVec3 localScaling = parent->getLocalScaling();
                        parentBoneData.RotationOffset = glm::vec3(static_cast<float>(rotationOffset.x), static_cast<float>(rotationOffset.y), static_cast<float>(rotationOffset.z));
                        parentBoneData.RotationPivot = glm::vec3(static_cast<float>(rotationPivot.x), static_cast<float>(rotationPivot.y), static_cast<float>(rotationPivot.z));
                        parentBoneData.PreRotation = glm::vec3(static_cast<float>(preRotation.x), static_cast<float>(preRotation.y), static_cast<float>(preRotation.z));
                        parentBoneData.PostRotation = glm::vec3(static_cast<float>(postRotation.x), static_cast<float>(postRotation.y), static_cast<float>(postRotation.z));
                        parentBoneData.ScalingOffset = glm::vec3(static_cast<float>(scalingOffset.x), static_cast<float>(scalingOffset.y), static_cast<float>(scalingOffset.z));
                        parentBoneData.ScalingPivot = glm::vec3(static_cast<float>(scalingPivot.x), static_cast<float>(scalingPivot.y), static_cast<float>(scalingPivot.z));
                        parentBoneData.LocalTranslation = glm::vec3(static_cast<float>(localTranslation.x), static_cast<float>(localTranslation.y), static_cast<float>(localTranslation.z));
                        parentBoneData.LocalRotation = glm::vec3(static_cast<float>(localRotation.x), static_cast<float>(localRotation.y), static_cast<float>(localRotation.z));
                        parentBoneData.LocalScaling = glm::vec3(static_cast<float>(localScaling.x), static_cast<float>(localScaling.y), static_cast<float>(localScaling.z));
                        parentBoneData.TransformLinkMatrix = parent->getGlobalTransform();
                        parentBoneData.TransformMatrix = parent->getGlobalTransform();

                        const int parentBoneIndex = static_cast<int>(currentFbxData->Bones.size());
                        currentFbxData->Bones.push_back(parentBoneData);
                        boneObjects.push_back(parent);
                        boneIndexByObject[parent] = parentBoneIndex;

                        parent = parent->getParent();
                    }
                }

                for (size_t boneIndex = 0; boneIndex < boneObjects.size(); ++boneIndex)
                {
                    const ofbx::Object* parent = boneObjects[boneIndex] ? boneObjects[boneIndex]->getParent() : nullptr;
                    if (!parent)
                        continue;

                    const auto parentIt = boneIndexByObject.find(parent);
                    if (parentIt != boneIndexByObject.end())
                        currentFbxData->Bones[boneIndex].ParentIndex = parentIt->second;
                }

                for (auto& vertexWeights : currentFbxData->VertexWeights)
                    NormalizeBoneInfluences(vertexWeights);

                size_t verticesWithoutInfluence = 0;
                size_t weightedVertices = 0;
                std::array<size_t, 4> influenceSlotsUsed{0, 0, 0, 0};
                for (const auto& vertexWeights : currentFbxData->VertexWeights)
                {
                    bool hasInfluence = false;
                    for (size_t slot = 0; slot < vertexWeights.Weights.size(); ++slot)
                    {
                        const float weight = vertexWeights.Weights[slot];
                        if (weight > 0.0f)
                        {
                            hasInfluence = true;
                            ++influenceSlotsUsed[slot];
                        }
                    }

                    if (hasInfluence)
                        ++weightedVertices;
                    else
                        ++verticesWithoutInfluence;
                }

                GPC_INFO << "[FBX DEBUG] Mesh='" << currentFbxData->MeshName.c_str()
                         << "' Node='" << currentFbxData->NodeName.c_str()
                         << "' Bones=" << currentFbxData->Bones.size()
                         << " Vertices=" << currentFbxData->VertexWeights.size()
                         << " WeightedVertices=" << weightedVertices
                         << " WithoutInfluence=" << verticesWithoutInfluence
                         << " SlotUsage=(" << influenceSlotsUsed[0] << ","
                         << influenceSlotsUsed[1] << ","
                         << influenceSlotsUsed[2] << ","
                         << influenceSlotsUsed[3] << ")"
                         << ENDL;
            }

            /// Transform
            ofbx::DVec3 localTrans = mesh->getLocalTranslation();
            ofbx::DVec3 localRot = mesh->getLocalRotation();
            ofbx::DVec3 localScale = mesh->getLocalScaling();
            currentFbxData->Position = glm::vec3(static_cast<float>(localTrans.x), static_cast<float>(localTrans.y), static_cast<float>(localTrans.z)) * 0.01f;
            currentFbxData->Rotation = glm::normalize(glm::quat(glm::radians(glm::vec3((float)localRot.x, (float)localRot.y, (float)localRot.z))));
            currentFbxData->Scale = glm::vec3(static_cast<float>(localScale.x), static_cast<float>(localScale.y), static_cast<float>(localScale.z));

            /// Texture
            const int materialCount = mesh->getMaterialCount();
            for (int m = 0; m < materialCount; ++m)
            {
                const ofbx::Material* material = mesh->getMaterial(m);
                if (!material)
                    continue;

                for (int texType = 0; texType < ofbx::Texture::COUNT; ++texType)
                {
                    const ofbx::Texture* texture = material->getTexture(static_cast<ofbx::Texture::TextureType>(texType));
                    if (!texture)
                        continue;

                    const ofbx::DataView filenameView = texture->getRelativeFileName();
                    std::string filename(filenameView.begin, filenameView.end);

                    const size_t textureSlashPos = filename.find_last_of("/\\");
                    if (textureSlashPos != std::string::npos)
                        filename = filename.substr(textureSlashPos + 1);

                    const size_t textureDotPos = filename.find_last_of('.');
                    if (textureDotPos == std::string::npos || textureDotPos == 0 || textureDotPos + 1 >= filename.size())
                        continue;

                    const auto textureType = static_cast<ofbx::Texture::TextureType>(texType);
                    if (currentFbxData->TextureFileNamesByType.find(textureType) == currentFbxData->TextureFileNamesByType.end())
                        currentFbxData->TextureFileNamesByType[textureType] = filename;
                }
            }


            if (i < meshCount - 1)
            {
                currentFbxData->pNext = CreateEmptyFbxMeshData();
                currentFbxData = currentFbxData->pNext;
            }
            else
            {
                currentFbxData->pNext = nullptr;
            }
        }
    }

    void LoadFbxDataToAssets(FbxMeshData* fbxData)
    {
        while (fbxData)
        {
            if (fbxData->pGeometryData)
                GPC_ASSETS->AddMesh(fbxData->MeshName, fbxData->pGeometryData);

            for (const auto& [texType, texFileName] : fbxData->TextureFileNamesByType)
            {
                std::string assetName = texFileName;
                const size_t extensionPos = assetName.find_last_of('.');
                if (extensionPos != std::string::npos && extensionPos > 0)
                    assetName = assetName.substr(0, extensionPos);

                GPC_ASSETS->AddTexture(assetName, "Resources/Materials/" + texFileName);
            }

            fbxData = fbxData->pNext;
        }
    }

    FbxLoader::FbxLoader() = default;
    FbxLoader::~FbxLoader() = default;

    FbxLoader* FbxLoader::GetInstance()
    {
        static FbxLoader instance;
        return &instance;
    }

    FbxMeshData* FbxLoader::Load(const std::string& name, const std::string& filePath, ofbx::LoadFlags flags)
    {
        FILE* fp = fopen(filePath.c_str(), "rb");
        if (!fp)
            return nullptr;

        fseek(fp, 0, SEEK_END);
        const long fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (fileSize <= 0)
        {
            fclose(fp);
            return nullptr;
        }

        auto* content = new ofbx::u8[fileSize];
        const size_t readSize = fread(content, 1, static_cast<size_t>(fileSize), fp);
        fclose(fp);

        if (readSize != static_cast<size_t>(fileSize))
        {
            delete[] content;
            return nullptr;
        }

        ofbx::IScene* scene = ofbx::load(content, fileSize, static_cast<ofbx::u16>(flags));
        if (!scene)
        {
            delete[] content;
            return nullptr;
        }

        auto* fbxData = CreateEmptyFbxMeshData();
        ExtractFbxData(scene, fbxData, name);
        LoadFbxAnimations(scene, fbxData);
        LoadFbxDataToAssets(fbxData);

        delete[] content;
        scene->destroy();

        return fbxData;
    }
} // namespace GPC
