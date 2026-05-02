#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Component.h"
#include "ECS_Defines.h"
#include "Geometry.h"
#include  "FbxLoader.h"

namespace GPC {

    class AnimationSystem;
    struct FbxAnimationData;
    struct FbxMeshData;

    struct CpuSkinVertexWeights {
        std::array<uint16_t, 4> BoneIndices{0, 0, 0, 0};
        std::array<float, 4> Weights{0.0f, 0.0f, 0.0f, 0.0f};
    };

    struct CpuRuntimeMeshData {
        std::vector<Vertex> Vertices{};
        std::vector<uint16_t> Indices{};
        bool VerticesDirty = false;
        bool IndicesDirty = false;
    };

    struct CpuSkinMeshInstance {
        Geometry* pGeometry = nullptr;

        std::vector<Vertex> BaseVertices{};
        std::vector<Vertex> SkinnedVertices{};
        std::vector<CpuSkinVertexWeights> VertexWeights{};

        std::vector<int> ParentIndices{};
        glm::mat4 MeshBindInverse{1.0f};
        std::vector<int> RotationOrders{};
        std::vector<glm::vec3> RotationOffsets{};
        std::vector<glm::vec3> RotationPivots{};
        std::vector<glm::vec3> PreRotations{};
        std::vector<glm::vec3> PostRotations{};
        std::vector<glm::vec3> ScalingOffsets{};
        std::vector<glm::vec3> ScalingPivots{};
        std::vector<glm::mat4> InverseBindMatrices{};
        std::vector<glm::vec3> BindLocalTranslations{};
        std::vector<glm::vec3> BindLocalRotations{};
        std::vector<glm::vec3> BindLocalScales{};

        std::unordered_map<std::string, uint32_t> BoneIndicesByName{};
        CpuRuntimeMeshData RuntimeMesh;
    };

    struct AnimationComponent : public Component {
        BIND_COMPONENT(ANIMATION);
        AUTO_COMPONENT_CONSTRUCTOR(AnimationComponent);

        std::string StackName{};
        uint32_t StackIndex = 0;
        double Time = 0.0;
        float Speed = 1.0f;
        bool Looping = true;
        bool Playing = true;
        bool EnableCpuSkinning = true;

        void SetFbxData(FbxMeshData* fbxData) {
            pFbxData = fbxData;
            pAnimationData = nullptr;
            RuntimeInitialized = false;
        }
        [[nodiscard]] bool HasFbxData() const { return pFbxData != nullptr; }
        [[nodiscard]] FbxMeshData* GetFbxData() const { return pFbxData; }
        [[nodiscard]] FbxAnimationData* GetAnimationData() const { return pAnimationData; }
        void SetAnimationData(FbxAnimationData* animationData) { pAnimationData = animationData; }
        [[nodiscard]] std::unordered_map<std::string, EntityID>& GetTargetEntities() { return TargetEntities; }
        [[nodiscard]] const std::unordered_map<std::string, EntityID>& GetTargetEntities() const { return TargetEntities; }
        [[nodiscard]] std::vector<CpuSkinMeshInstance>& GetCpuSkins() { return CpuSkins; }
        [[nodiscard]] std::vector<std::shared_ptr<Geometry>>& GetRuntimeGeometries() { return RuntimeGeometries; }
        [[nodiscard]] bool IsRuntimeInitialized() const { return RuntimeInitialized; }
        void SetRuntimeInitialized(bool value) { RuntimeInitialized = value; }
        void ChangeAnimationTo(const std::string& stackName) {
            StackName = stackName;
            Time = 0.0;
            Playing = true;
        }
        void ChangeAnimationTo(int stackIndex) {
            StackIndex = stackIndex;
            StackName = pFbxData->pAnimationData->Stacks[StackIndex].Name;
            Time = 0.0;
            Playing = true;
        }

    private:
        friend class AnimationSystem;

        FbxMeshData* pFbxData = nullptr;
        FbxAnimationData* pAnimationData = nullptr;
        std::unordered_map<std::string, EntityID> TargetEntities{};
        bool RuntimeInitialized = false;
        std::vector<CpuSkinMeshInstance> CpuSkins{};
        std::vector<std::shared_ptr<Geometry>> RuntimeGeometries{};
    };

} // GPC