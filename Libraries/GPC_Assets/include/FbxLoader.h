#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "GPC_Framework.h"
#include "Assets.h"

#define GPC_FBX GPC::FbxLoader::GetInstance()

namespace GPC
{

    enum class FbxAnimationProperty
    {
        Unknown,
        Translation,
        Rotation,
        Scaling,
    };

    struct FbxBoneData
    {
        std::string Name;
        int ParentIndex = -1;
        int RotationOrder = static_cast<int>(ofbx::RotationOrder::EULER_XYZ);
        glm::vec3 RotationOffset{0.0f};
        glm::vec3 RotationPivot{0.0f};
        glm::vec3 PreRotation{0.0f};
        glm::vec3 PostRotation{0.0f};
        glm::vec3 ScalingOffset{0.0f};
        glm::vec3 ScalingPivot{0.0f};
        glm::vec3 LocalTranslation{0.0f};
        glm::vec3 LocalRotation{0.0f};
        glm::vec3 LocalScaling{1.0f};
        ofbx::DMatrix TransformMatrix{};
        ofbx::DMatrix TransformLinkMatrix{};
    };

    struct FbxVertexWeights
    {
        std::array<uint16_t, 4> BoneIndices{0, 0, 0, 0};
        std::array<float, 4> Weights{0.0f, 0.0f, 0.0f, 0.0f};
    };

    struct FbxAnimationKey
    {
        double Time = 0.0;
        float Value = 0.0f;
    };

    struct FbxAnimationCurveData
    {
        std::vector<FbxAnimationKey> Keys;
    };

    struct FbxAnimationNodeData
    {
        std::string TargetName;
        FbxAnimationProperty Property = FbxAnimationProperty::Unknown;
        std::array<FbxAnimationCurveData, 3> Curves;
    };

    struct FbxAnimationLayerData
    {
        std::string Name;
        std::vector<FbxAnimationNodeData> Nodes;
    };

    struct FbxAnimationStackData
    {
        std::string Name;
        double StartTime = 0.0;
        double EndTime = 0.0;
        std::vector<FbxAnimationLayerData> Layers;
    };

    struct FbxAnimationData
    {
        std::vector<FbxAnimationStackData> Stacks;
    };

    struct FbxMeshData
    {
        std::string MeshName;
        std::string NodeName;
        std::unordered_map<ofbx::Texture::TextureType, std::string> TextureFileNamesByType;
        GeometryData* pGeometryData;

        glm::vec3 Position;
        glm::quat Rotation;
        glm::vec3 Scale;

        std::vector<FbxBoneData> Bones;
        std::vector<FbxVertexWeights> VertexWeights;
        bool HasSkin = false;

        FbxAnimationData* pAnimationData = nullptr;

        FbxMeshData* pNext;
    };

    class FbxLoader
    {
    public:
        FbxLoader();
        ~FbxLoader();

        static FbxLoader* GetInstance();
        FbxMeshData* Load(const std::string& name, const std::string& filePath, ofbx::LoadFlags flags = ofbx::LoadFlags::NONE);
    };
}