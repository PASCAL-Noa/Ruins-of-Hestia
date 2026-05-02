#include "AnimationSystem.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

#include "FbxLoader.h"
#include "Render3DSystem.h"
#include "Scene.h"
#include "TransformComponents.h"

namespace GPC {

    namespace {
        std::string NormalizeAnimationTargetName(const std::string& name) {
            if (name.empty()) {
                return {};
            }

            std::string normalized = name;
            const size_t namespacePos = normalized.find_last_of(':');
            if (namespacePos != std::string::npos && namespacePos + 1 < normalized.size()) {
                normalized = normalized.substr(namespacePos + 1);
            }

            const size_t hierarchyPos = normalized.find_last_of('|');
            if (hierarchyPos != std::string::npos && hierarchyPos + 1 < normalized.size()) {
                normalized = normalized.substr(hierarchyPos + 1);
            }

            return normalized;
        }

        bool FindTargetEntity(const AnimationComponent& animation, const std::string& targetName, EntityID& outEntity) {
            const auto& targetEntities = animation.GetTargetEntities();

            const auto exactIt = targetEntities.find(targetName);
            if (exactIt != targetEntities.end()) {
                outEntity = exactIt->second;
                return true;
            }

            const std::string normalizedTarget = NormalizeAnimationTargetName(targetName);
            if (normalizedTarget.empty()) {
                return false;
            }

            const auto normalizedIt = targetEntities.find(normalizedTarget);
            if (normalizedIt != targetEntities.end()) {
                outEntity = normalizedIt->second;
                return true;
            }

            for (const auto& [name, eid] : targetEntities) {
                if (NormalizeAnimationTargetName(name) == normalizedTarget) {
                    outEntity = eid;
                    return true;
                }
            }

            return false;
        }

        bool FindBoneIndex(const CpuSkinMeshInstance& skin, const std::string& targetName, uint32_t& outBoneIndex) {
            const auto exactIt = skin.BoneIndicesByName.find(targetName);
            if (exactIt != skin.BoneIndicesByName.end()) {
                outBoneIndex = exactIt->second;
                return true;
            }

            const std::string normalizedTarget = NormalizeAnimationTargetName(targetName);
            if (normalizedTarget.empty()) {
                return false;
            }

            const auto normalizedIt = skin.BoneIndicesByName.find(normalizedTarget);
            if (normalizedIt != skin.BoneIndicesByName.end()) {
                outBoneIndex = normalizedIt->second;
                return true;
            }

            return false;
        }

        glm::mat4 RotationMatrixFromEuler(const glm::vec3& eulerDeg, ofbx::RotationOrder order) {
            const glm::vec3 eulerRad = glm::radians(eulerDeg);
            const glm::mat4 rx = glm::rotate(glm::mat4(1.0f), eulerRad.x, glm::vec3(1.0f, 0.0f, 0.0f));
            const glm::mat4 ry = glm::rotate(glm::mat4(1.0f), eulerRad.y, glm::vec3(0.0f, 1.0f, 0.0f));
            const glm::mat4 rz = glm::rotate(glm::mat4(1.0f), eulerRad.z, glm::vec3(0.0f, 0.0f, 1.0f));

            switch (order) {
                default:
                case ofbx::RotationOrder::EULER_XYZ: return rz * ry * rx;
                case ofbx::RotationOrder::EULER_XZY: return ry * rz * rx;
                case ofbx::RotationOrder::EULER_YZX: return rx * rz * ry;
                case ofbx::RotationOrder::EULER_YXZ: return rz * rx * ry;
                case ofbx::RotationOrder::EULER_ZXY: return ry * rx * rz;
                case ofbx::RotationOrder::EULER_ZYX: return rx * ry * rz;
                case ofbx::RotationOrder::SPHERIC_XYZ: return rz * ry * rx;
            }
        }

        glm::mat4 ComposeFbxLocalMatrix(
            const glm::vec3& translation,
            const glm::vec3& rotationDeg,
            const glm::vec3& scale,
            int rotationOrder,
            const glm::vec3& rotationOffset,
            const glm::vec3& rotationPivot,
            const glm::vec3& preRotation,
            const glm::vec3& postRotation,
            const glm::vec3& scalingOffset,
            const glm::vec3& scalingPivot) {
            const glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
            const glm::mat4 r_off = glm::translate(glm::mat4(1.0f), rotationOffset);
            const glm::mat4 r_p = glm::translate(glm::mat4(1.0f), rotationPivot);
            const glm::mat4 r_p_inv = glm::translate(glm::mat4(1.0f), -rotationPivot);
            const glm::mat4 s_off = glm::translate(glm::mat4(1.0f), scalingOffset);
            const glm::mat4 s_p = glm::translate(glm::mat4(1.0f), scalingPivot);
            const glm::mat4 s_p_inv = glm::translate(glm::mat4(1.0f), -scalingPivot);
            const glm::mat4 r_pre = RotationMatrixFromEuler(preRotation, ofbx::RotationOrder::EULER_XYZ);
            const glm::mat4 r_post_inv = RotationMatrixFromEuler(-postRotation, ofbx::RotationOrder::EULER_ZYX);
            const glm::mat4 r = RotationMatrixFromEuler(rotationDeg, static_cast<ofbx::RotationOrder>(rotationOrder));
            const glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);

            return t * r_off * r_p * r_pre * r * r_post_inv * r_p_inv * s_off * s_p * s * s_p_inv;
        }

        glm::mat4 ResolveGlobalBoneMatrix(
            const size_t boneIndex,
            const std::vector<int>& parentIndices,
            const std::vector<glm::mat4>& localMatrices,
            std::vector<glm::mat4>& globalMatrices,
            std::vector<uint8_t>& visitState)
        {
            // 0 = not visited, 1 = visiting, 2 = resolved
            if (visitState[boneIndex] == 2) {
                return globalMatrices[boneIndex];
            }

            if (visitState[boneIndex] == 1) {
                globalMatrices[boneIndex] = localMatrices[boneIndex];
                visitState[boneIndex] = 2;
                return globalMatrices[boneIndex];
            }

            visitState[boneIndex] = 1;

            const int parentIndex = parentIndices[boneIndex];
            if (parentIndex >= 0 && static_cast<size_t>(parentIndex) < localMatrices.size()) {
                const glm::mat4 parentGlobal = ResolveGlobalBoneMatrix(
                    static_cast<size_t>(parentIndex),
                    parentIndices,
                    localMatrices,
                    globalMatrices,
                    visitState);
                globalMatrices[boneIndex] = parentGlobal * localMatrices[boneIndex];
            } else {
                globalMatrices[boneIndex] = localMatrices[boneIndex];
            }

            visitState[boneIndex] = 2;
            return globalMatrices[boneIndex];
        }

        glm::mat4 OfbxMatrixToGlm(const ofbx::DMatrix& matrix) {
            glm::mat4 out(1.0f);
            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 4; ++col) {
                    out[col][row] = static_cast<float>(matrix.m[col * 4 + row]);
                }
            }
            return out;
        }

        glm::mat4 BuildTransformMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) {
            return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
        }

        void BuildCpuSkinMeshFromFbx(const FbxMeshData* currentFbxData, Geometry* runtimeGeometry, CpuSkinMeshInstance& outSkin) {
            outSkin.pGeometry = runtimeGeometry;
            outSkin.BaseVertices = currentFbxData->pGeometryData->Vertices;
            outSkin.SkinnedVertices = outSkin.BaseVertices;
            outSkin.RuntimeMesh.Vertices = outSkin.BaseVertices;
            outSkin.RuntimeMesh.Indices = currentFbxData->pGeometryData->Indices;

            outSkin.VertexWeights.resize(currentFbxData->VertexWeights.size());
            for (size_t i = 0; i < currentFbxData->VertexWeights.size(); ++i) {
                outSkin.VertexWeights[i].BoneIndices = currentFbxData->VertexWeights[i].BoneIndices;
                outSkin.VertexWeights[i].Weights = currentFbxData->VertexWeights[i].Weights;

                float sum = 0.0f;
                for (float& weight : outSkin.VertexWeights[i].Weights) {
                    if (!std::isfinite(weight) || weight < 0.0f) {
                        weight = 0.0f;
                    }
                    sum += weight;
                }

                if (sum > 0.00001f) {
                    const float invSum = 1.0f / sum;
                    for (float& weight : outSkin.VertexWeights[i].Weights) {
                        weight *= invSum;
                    }
                } else {
                    outSkin.VertexWeights[i].BoneIndices = {0, 0, 0, 0};
                    outSkin.VertexWeights[i].Weights = {0.0f, 0.0f, 0.0f, 0.0f};
                }
            }

            const size_t boneCount = currentFbxData->Bones.size();
            const glm::mat4 meshBindGlobal = BuildTransformMatrix(currentFbxData->Position, currentFbxData->Rotation, currentFbxData->Scale);
            outSkin.MeshBindInverse = glm::inverse(meshBindGlobal);
            outSkin.ParentIndices.reserve(boneCount);
            outSkin.RotationOrders.reserve(boneCount);
            outSkin.RotationOffsets.reserve(boneCount);
            outSkin.RotationPivots.reserve(boneCount);
            outSkin.PreRotations.reserve(boneCount);
            outSkin.PostRotations.reserve(boneCount);
            outSkin.ScalingOffsets.reserve(boneCount);
            outSkin.ScalingPivots.reserve(boneCount);
            outSkin.InverseBindMatrices.reserve(boneCount);
            outSkin.BindLocalTranslations.reserve(boneCount);
            outSkin.BindLocalRotations.reserve(boneCount);
            outSkin.BindLocalScales.reserve(boneCount);

            std::vector<glm::mat4> bindGlobalMatrices(boneCount, glm::mat4(1.0f));
            for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
                const FbxBoneData& bone = currentFbxData->Bones[boneIndex];
                outSkin.ParentIndices.push_back(bone.ParentIndex);
                outSkin.RotationOrders.push_back(bone.RotationOrder);
                outSkin.RotationOffsets.push_back(bone.RotationOffset);
                outSkin.RotationPivots.push_back(bone.RotationPivot);
                outSkin.PreRotations.push_back(bone.PreRotation);
                outSkin.PostRotations.push_back(bone.PostRotation);
                outSkin.ScalingOffsets.push_back(bone.ScalingOffset);
                outSkin.ScalingPivots.push_back(bone.ScalingPivot);

                const std::string normalizedBoneName = NormalizeAnimationTargetName(bone.Name);
                outSkin.BoneIndicesByName[bone.Name] = static_cast<uint32_t>(boneIndex);
                if (!normalizedBoneName.empty()) {
                    outSkin.BoneIndicesByName[normalizedBoneName] = static_cast<uint32_t>(boneIndex);
                }

                const glm::mat4 boneBindGlobal = OfbxMatrixToGlm(bone.TransformLinkMatrix);
                bindGlobalMatrices[boneIndex] = outSkin.MeshBindInverse * boneBindGlobal;
                outSkin.InverseBindMatrices.push_back(glm::inverse(bindGlobalMatrices[boneIndex]));
            }

            for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
                const FbxBoneData& bone = currentFbxData->Bones[boneIndex];
                outSkin.BindLocalTranslations.push_back(bone.LocalTranslation);
                outSkin.BindLocalRotations.push_back(bone.LocalRotation);
                outSkin.BindLocalScales.push_back(bone.LocalScaling);
            }
        }

        void InitializeAnimationRuntime(AnimationComponent& animation, Scene* scene) {
            if (animation.IsRuntimeInitialized() || !scene || !animation.HasFbxData()) {
                return;
            }

            if (!animation.GetAnimationData()) {
                animation.SetAnimationData(animation.GetFbxData()->pAnimationData);
            }

            animation.GetTargetEntities().clear();
            animation.GetCpuSkins().clear();
            animation.GetRuntimeGeometries().clear();

            CBlockPtr<Render3DComponent> renderBlock = scene->GetComponentBlock<Render3DComponent>();
            Render3DComponent* renderers = renderBlock->ComponentData();
            EntityID* rendererEntities = renderBlock->EntitiesData();

            std::unordered_map<Geometry*, std::vector<EntityID>> entitiesBySourceGeometry;
            std::unordered_map<Geometry*, size_t> consumeIndexByGeometry;
            for (uint32_t i = 0; i < renderBlock->GetEntityCount(); ++i) {
                if (renderers[i].pGeometry) {
                    entitiesBySourceGeometry[renderers[i].pGeometry].push_back(rendererEntities[i]);
                }
            }

            for (FbxMeshData* currentFbxData = animation.GetFbxData(); currentFbxData; currentFbxData = currentFbxData->pNext) {
                EntityID targetEntity = MAX_ENTITIES;
                Geometry* sourceGeometry = currentFbxData->MeshName.empty() ? nullptr : GPC_MESH(currentFbxData->MeshName);
                if (sourceGeometry) {
                    const auto entitiesIt = entitiesBySourceGeometry.find(sourceGeometry);
                    if (entitiesIt != entitiesBySourceGeometry.end()) {
                        size_t& consumeIndex = consumeIndexByGeometry[sourceGeometry];
                        if (consumeIndex < entitiesIt->second.size()) {
                            targetEntity = entitiesIt->second[consumeIndex++];
                        }
                    }
                }

                if (targetEntity != MAX_ENTITIES) {
                    const std::string normalizedNodeName = NormalizeAnimationTargetName(currentFbxData->NodeName);
                    if (!currentFbxData->NodeName.empty()) {
                        animation.GetTargetEntities()[currentFbxData->NodeName] = targetEntity;
                    }
                    if (!normalizedNodeName.empty()) {
                        animation.GetTargetEntities()[normalizedNodeName] = targetEntity;
                    }
                    if (!currentFbxData->MeshName.empty()) {
                        animation.GetTargetEntities()[currentFbxData->MeshName] = targetEntity;
                    }
                }

                if (!animation.EnableCpuSkinning || targetEntity == MAX_ENTITIES || !currentFbxData->HasSkin || !currentFbxData->pGeometryData) {
                    continue;
                }

                auto runtimeGeometry = std::make_shared<Geometry>();
                if (GPC_FAILED(runtimeGeometry->Create(currentFbxData->pGeometryData))) {
                    continue;
                }

                Render3DComponent* renderer = scene->GetComponent<Render3DComponent>(targetEntity);
                if (renderer) {
                    renderer->pGeometry = runtimeGeometry.get();
                }

                CpuSkinMeshInstance skin;
                BuildCpuSkinMeshFromFbx(currentFbxData, runtimeGeometry.get(), skin);

                animation.GetRuntimeGeometries().push_back(runtimeGeometry);
                animation.GetCpuSkins().push_back(std::move(skin));
            }

            if (animation.GetAnimationData() && !animation.GetAnimationData()->Stacks.empty()) {
                animation.StackIndex = std::min<uint32_t>(animation.StackIndex, static_cast<uint32_t>(animation.GetAnimationData()->Stacks.size() - 1));
                if (animation.StackName.empty()) {
                    animation.StackName = animation.GetAnimationData()->Stacks[animation.StackIndex].Name;
                    animation.Time = animation.GetAnimationData()->Stacks[animation.StackIndex].StartTime;
                }

                size_t meshCount = 0;
                size_t boneCount = 0;
                for (FbxMeshData* current = animation.GetFbxData(); current; current = current->pNext) {
                    ++meshCount;
                    boneCount += current->Bones.size();
                }

                GPC_INFO << "[FBX DEBUG] ActiveStack index=" << animation.StackIndex
                         << " name='" << animation.StackName.c_str() << "'"
                         << " meshes=" << meshCount
                         << " bones=" << boneCount
                         << " cpuSkinning=" << (animation.EnableCpuSkinning ? "true" : "false")
                         << ENDL;

                if (!animation.GetCpuSkins().empty()) {
                    const CpuSkinMeshInstance& skin = animation.GetCpuSkins().front();
                    const size_t skinBoneCount = skin.ParentIndices.size();
                    if (skinBoneCount > 0 && skin.InverseBindMatrices.size() == skinBoneCount &&
                        skin.BindLocalTranslations.size() == skinBoneCount &&
                        skin.BindLocalRotations.size() == skinBoneCount &&
                        skin.BindLocalScales.size() == skinBoneCount) {

                        std::vector<glm::mat4> localMatrices(skinBoneCount, glm::mat4(1.0f));
                        std::vector<glm::mat4> globalMatrices(skinBoneCount, glm::mat4(1.0f));
                        std::vector<glm::mat4> skinMatrices(skinBoneCount, glm::mat4(1.0f));
                        std::vector<uint8_t> visitState(skinBoneCount, 0);

                        for (size_t bone = 0; bone < skinBoneCount; ++bone) {
                            localMatrices[bone] = ComposeFbxLocalMatrix(
                                skin.BindLocalTranslations[bone],
                                skin.BindLocalRotations[bone],
                                skin.BindLocalScales[bone],
                                skin.RotationOrders[bone],
                                skin.RotationOffsets[bone],
                                skin.RotationPivots[bone],
                                skin.PreRotations[bone],
                                skin.PostRotations[bone],
                                skin.ScalingOffsets[bone],
                                skin.ScalingPivots[bone]);
                        }

                        float maxDeviation = 0.0f;
                        for (size_t bone = 0; bone < skinBoneCount; ++bone) {
                            const glm::mat4 globalMatrix = ResolveGlobalBoneMatrix(
                                bone,
                                skin.ParentIndices,
                                localMatrices,
                                globalMatrices,
                                visitState);
                            const glm::mat4 meshLocalGlobal = skin.MeshBindInverse * globalMatrix;
                            skinMatrices[bone] = meshLocalGlobal * skin.InverseBindMatrices[bone];

                            for (int row = 0; row < 4; ++row) {
                                for (int col = 0; col < 4; ++col) {
                                    const float expected = (row == col) ? 1.0f : 0.0f;
                                    const float delta = std::abs(skinMatrices[bone][col][row] - expected);
                                    if (delta > maxDeviation) {
                                        maxDeviation = delta;
                                    }
                                }
                            }
                        }

                        GPC_INFO << "[FBX DEBUG] BindPose maxDeviation=" << maxDeviation
                                 << " firstSkinBones=" << skinBoneCount
                                 << ENDL;
                    }

                    const FbxAnimationStackData& activeStack = animation.GetAnimationData()->Stacks[animation.StackIndex];
                    size_t totalNodes = 0;
                    size_t matchedNodes = 0;
                    size_t unmatchedNodes = 0;
                    std::array<std::string, 5> firstUnmatched{};
                    size_t firstUnmatchedCount = 0;

                    for (const FbxAnimationLayerData& layer : activeStack.Layers) {
                        for (const FbxAnimationNodeData& node : layer.Nodes) {
                            if (node.TargetName.empty()) {
                                continue;
                            }

                            ++totalNodes;
                            uint32_t boneIndex = 0;
                            if (FindBoneIndex(skin, node.TargetName, boneIndex) && boneIndex < skinBoneCount) {
                                ++matchedNodes;
                            } else {
                                ++unmatchedNodes;
                                if (firstUnmatchedCount < firstUnmatched.size()) {
                                    firstUnmatched[firstUnmatchedCount++] = node.TargetName;
                                }
                            }
                        }
                    }

                    GPC_INFO << "[FBX DEBUG] NodeMapping stack='" << activeStack.Name.c_str()
                             << "' matched=" << matchedNodes
                             << "/" << totalNodes
                             << " unmatched=" << unmatchedNodes;
                    if (firstUnmatchedCount > 0) {
                        GPC_INFO << " firstUnmatched=[";
                        for (size_t idx = 0; idx < firstUnmatchedCount; ++idx) {
                            if (idx > 0) {
                                GPC_INFO << ", ";
                            }
                            GPC_INFO << firstUnmatched[idx].c_str();
                        }
                        GPC_INFO << "]";
                    }
                    GPC_INFO << ENDL;
                }
            }

            animation.SetRuntimeInitialized(true);
        }

        float SampleCurveValue(const FbxAnimationCurveData& curveData, const double time, float fallback) {
            if (curveData.Keys.empty()) {
                return fallback;
            }

            if (time <= curveData.Keys.front().Time) {
                return curveData.Keys.front().Value;
            }

            if (time >= curveData.Keys.back().Time) {
                return curveData.Keys.back().Value;
            }

            auto upper = std::lower_bound(
                curveData.Keys.begin(),
                curveData.Keys.end(),
                time,
                [](const FbxAnimationKey& key, const double value) { return key.Time < value; }
            );

            if (upper == curveData.Keys.begin() || upper == curveData.Keys.end()) {
                return upper->Value;
            }

            const FbxAnimationKey& next = *upper;
            const FbxAnimationKey& prev = *(upper - 1);
            const double range = next.Time - prev.Time;
            if (range <= 0.0) {
                return prev.Value;
            }

            const float alpha = static_cast<float>((time - prev.Time) / range);
            return glm::mix(prev.Value, next.Value, alpha);
        }

        glm::vec3 SampleNodeVector3(const FbxAnimationNodeData& nodeData, const double time, const glm::vec3& fallback) {
            glm::vec3 sampled = fallback;
            sampled.x = SampleCurveValue(nodeData.Curves[0], time, sampled.x);
            sampled.y = SampleCurveValue(nodeData.Curves[1], time, sampled.y);
            sampled.z = SampleCurveValue(nodeData.Curves[2], time, sampled.z);
            return sampled;
        }

        const FbxAnimationStackData* ResolveStack(const AnimationComponent& animation) {
            if (!animation.GetAnimationData()) {
                return nullptr;
            }

            const auto& stacks = animation.GetAnimationData()->Stacks;
            if (stacks.empty()) {
                return nullptr;
            }

            if (!animation.StackName.empty()) {
                for (const auto& stack : stacks) {
                    if (stack.Name == animation.StackName) {
                        return &stack;
                    }
                }
            }

            const size_t index = std::min(static_cast<size_t>(animation.StackIndex), stacks.size() - 1);
            return &stacks[index];
        }
    }

    void AnimationSystem::OnUpdate()
    {
        System::OnUpdate();

        CBlockPtr<AnimationComponent> animationBlock = m_Ctx.pScene->GetComponentBlock<AnimationComponent>();
        AnimationComponent* animations = animationBlock->ComponentData();
        const uint32_t animationCount = animationBlock->GetEntityCount();
        const double deltaTime = m_Ctx.pClock->GetDeltaTime();

        for (uint32_t i = 0; i < animationCount; ++i)
        {
            AnimationComponent& animation = animations[i];
            if (!animation.IsEnable || !animation.Playing) {
                continue;
            }

            InitializeAnimationRuntime(animation, m_Ctx.pScene);
            if (!animation.GetAnimationData()) {
                continue;
            }

            const FbxAnimationStackData* stack = ResolveStack(animation);
            if (!stack || stack->Layers.empty()) {
                continue;
            }

            const double startTime = stack->StartTime;
            const double endTime = stack->EndTime;
            const double duration = endTime - startTime;

            animation.Time += deltaTime * static_cast<double>(animation.Speed);

            double sampledTime = animation.Time;
            if (duration > 0.0) {
                if (animation.Looping) {
                    sampledTime = startTime + std::fmod(animation.Time - startTime, duration);
                    if (sampledTime < startTime) {
                        sampledTime += duration;
                    }
                } else {
                    sampledTime = std::clamp(animation.Time, startTime, endTime);
                    if (sampledTime >= endTime) {
                        animation.Playing = false;
                    }
                }
            }

            const bool hasCpuSkins = animation.EnableCpuSkinning && !animation.GetCpuSkins().empty();
            if (!hasCpuSkins) {
                for (const FbxAnimationLayerData& layer : stack->Layers)
                {
                    for (const FbxAnimationNodeData& node : layer.Nodes)
                    {
                        if (node.TargetName.empty()) {
                            continue;
                        }

                        EntityID targetEntity = MAX_ENTITIES;
                        if (!FindTargetEntity(animation, node.TargetName, targetEntity)) {
                            continue;
                        }

                        Transform3D* transform = m_Ctx.pScene->GetComponent<Transform3D>(targetEntity);
                        if (!transform) {
                            continue;
                        }

                        switch (node.Property)
                        {
                            case FbxAnimationProperty::Translation:
                            {
                                const glm::vec3 position = SampleNodeVector3(node, sampledTime, transform->LocalTransform.GetPosition());
                                // FBX stores translations in centimeters in this loader path.
                                transform->LocalTransform.SetPosition(position * 0.01f);
                                break;
                            }
                            case FbxAnimationProperty::Scaling:
                            {
                                const glm::vec3 scale = SampleNodeVector3(node, sampledTime, transform->LocalTransform.GetScale());
                                transform->LocalTransform.SetScale(scale);
                                break;
                            }
                            case FbxAnimationProperty::Rotation:
                            {
                                const glm::vec3 baseEulerDeg = glm::degrees(glm::eulerAngles(transform->LocalTransform.GetRotation()));
                                const glm::vec3 eulerDeg = SampleNodeVector3(node, sampledTime, baseEulerDeg);
                                transform->LocalTransform.SetRotation(glm::quat(glm::radians(eulerDeg)));
                                break;
                            }
                            case FbxAnimationProperty::Unknown:
                            default:
                                break;
                        }
                    }
                }
            }

            if (!animation.EnableCpuSkinning) {
                continue;
            }

            for (CpuSkinMeshInstance& skin : animation.GetCpuSkins())
            {
                if (!skin.pGeometry || skin.BaseVertices.empty() || skin.VertexWeights.size() != skin.BaseVertices.size()) {
                    continue;
                }

                const size_t boneCount = skin.ParentIndices.size();
                if (boneCount == 0 || skin.InverseBindMatrices.size() != boneCount ||
                    skin.BindLocalTranslations.size() != boneCount ||
                    skin.BindLocalRotations.size() != boneCount ||
                    skin.BindLocalScales.size() != boneCount) {
                    continue;
                }

                std::vector<glm::vec3> localTranslations = skin.BindLocalTranslations;
                std::vector<glm::vec3> localRotations = skin.BindLocalRotations;
                std::vector<glm::vec3> localScales = skin.BindLocalScales;

                for (const FbxAnimationLayerData& layer : stack->Layers)
                {
                    for (const FbxAnimationNodeData& node : layer.Nodes)
                    {
                        if (node.TargetName.empty()) {
                            continue;
                        }

                        uint32_t boneIndex = 0;
                        if (!FindBoneIndex(skin, node.TargetName, boneIndex) || boneIndex >= boneCount) {
                            continue;
                        }

                        switch (node.Property)
                        {
                            case FbxAnimationProperty::Translation:
                            {
                                const glm::vec3 localPosition = SampleNodeVector3(node, sampledTime, localTranslations[boneIndex]);
                                // Les bones skinnes restent dans l'espace FBX brut pour rester coherents avec
                                // les vertices et les matrices de bind; la conversion metres est reservee aux
                                // entites non skinnes.
                                localTranslations[boneIndex] = localPosition;
                                break;
                            }
                            case FbxAnimationProperty::Scaling:
                            {
                                localScales[boneIndex] = SampleNodeVector3(node, sampledTime, localScales[boneIndex]);
                                break;
                            }
                            case FbxAnimationProperty::Rotation:
                            {
                                localRotations[boneIndex] = SampleNodeVector3(node, sampledTime, localRotations[boneIndex]);
                                break;
                            }
                            case FbxAnimationProperty::Unknown:
                            default:
                                break;
                        }
                    }
                }

                std::vector<glm::mat4> localMatrices(boneCount, glm::mat4(1.0f));
                std::vector<glm::mat4> globalMatrices(boneCount, glm::mat4(1.0f));
                std::vector<glm::mat4> skinMatrices(boneCount, glm::mat4(1.0f));
                std::vector<uint8_t> visitState(boneCount, 0);

                for (size_t bone = 0; bone < boneCount; ++bone)
                {
                    localMatrices[bone] = ComposeFbxLocalMatrix(
                        localTranslations[bone],
                        localRotations[bone],
                        localScales[bone],
                        skin.RotationOrders[bone],
                        skin.RotationOffsets[bone],
                        skin.RotationPivots[bone],
                        skin.PreRotations[bone],
                        skin.PostRotations[bone],
                        skin.ScalingOffsets[bone],
                        skin.ScalingPivots[bone]);
                }

                for (size_t bone = 0; bone < boneCount; ++bone)
                {
                    const glm::mat4 globalMatrix = ResolveGlobalBoneMatrix(
                        bone,
                        skin.ParentIndices,
                        localMatrices,
                        globalMatrices,
                        visitState);
                    const glm::mat4 meshLocalGlobal = skin.MeshBindInverse * globalMatrix;
                    skinMatrices[bone] = meshLocalGlobal * skin.InverseBindMatrices[bone];
                }

                skin.SkinnedVertices = skin.BaseVertices;

                for (size_t vertexIndex = 0; vertexIndex < skin.SkinnedVertices.size(); ++vertexIndex)
                {
                    const Vertex& baseVertex = skin.BaseVertices[vertexIndex];
                    Vertex& skinnedVertex = skin.SkinnedVertices[vertexIndex];
                    const CpuSkinVertexWeights& influences = skin.VertexWeights[vertexIndex];

                    glm::vec3 skinnedPosition(0.0f);
                    glm::vec3 skinnedNormal(0.0f);
                    float validWeightSum = 0.0f;

                    for (size_t influence = 0; influence < influences.Weights.size(); ++influence)
                    {
                        const float weight = influences.Weights[influence];
                        if (!std::isfinite(weight) || weight <= 0.0f) {
                            continue;
                        }

                        const uint32_t boneIndex = influences.BoneIndices[influence];
                        if (boneIndex >= skinMatrices.size()) {
                            continue;
                        }

                        const glm::mat4& skinMatrix = skinMatrices[boneIndex];
                        skinnedPosition += glm::vec3(skinMatrix * glm::vec4(baseVertex.Position, 1.0f)) * weight;
                        skinnedNormal += glm::mat3(skinMatrix) * baseVertex.Normals * weight;
                        validWeightSum += weight;
                    }

                    if (validWeightSum > 0.00001f) {
                        const float invWeight = 1.0f / validWeightSum;
                        skinnedVertex.Position = skinnedPosition * invWeight;

                        const glm::vec3 renormalizedNormal = skinnedNormal * invWeight;
                        if (glm::length(renormalizedNormal) > 0.0f) {
                            skinnedVertex.Normals = glm::normalize(renormalizedNormal);
                        } else {
                            skinnedVertex.Normals = baseVertex.Normals;
                        }
                    } else {
                        // Aucun os valide pour ce vertex: on garde la bind-pose pour eviter les pics.
                        skinnedVertex.Position = baseVertex.Position;
                        skinnedVertex.Normals = baseVertex.Normals;
                    }

                    if (!std::isfinite(skinnedVertex.Position.x) || !std::isfinite(skinnedVertex.Position.y) || !std::isfinite(skinnedVertex.Position.z)) {
                        skinnedVertex.Position = baseVertex.Position;
                    }
                }

                skin.RuntimeMesh.Vertices = skin.SkinnedVertices;
                skin.RuntimeMesh.VerticesDirty = true;

                if (skin.RuntimeMesh.VerticesDirty || skin.RuntimeMesh.IndicesDirty) {
                    ErrorType updateResult = ErrorType::SUCCESS;

                    if (skin.RuntimeMesh.IndicesDirty) {
                        if (skin.RuntimeMesh.VerticesDirty) {
                            updateResult = skin.pGeometry->UpdateMeshData(skin.RuntimeMesh.Vertices, skin.RuntimeMesh.Indices);
                        } else {
                            updateResult = skin.pGeometry->UpdateIndices(skin.RuntimeMesh.Indices);
                        }
                    } else {
                        updateResult = skin.pGeometry->UpdateVertices(skin.RuntimeMesh.Vertices);
                    }

                    if (!GPC_FAILED(updateResult)) {
                        skin.RuntimeMesh.VerticesDirty = false;
                        skin.RuntimeMesh.IndicesDirty = false;
                    }
                }
            }
        }
    }
} // GPC