#pragma once
#include "AnimationComponent.h"
#include "Behavior.h"
#include "FbxLoader.h"
#include "Render3DSystem.h"

namespace GPC {


    struct IrisFollowing : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(IrisFollowing);

        FbxMeshData* pIrisMesh = nullptr;
        Transform3D* pPlayerTransform = nullptr;

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

    private:
        static constexpr float COOLDOWN_BETWEEN_UPDATE = 0.1f;
        static constexpr float IRIS_SPEED = 0.2 * GENERAL_SCALE_EXPEDITION;
        static constexpr float IRIS_MIN_DISTANCE_TO_CALI = 0.25f * GENERAL_SCALE_EXPEDITION;
        static constexpr float MIN_DISTANCE_TO_BE_REGISTERED = 0.5f * GENERAL_SCALE_EXPEDITION;
        static constexpr float DISTANCE_TO_NODE_TO_ACQUIRED = 0.25f * GENERAL_SCALE_EXPEDITION;
        static constexpr uint32_t MIN_REGISTERED_BEFORE_FOLLOWING = 2;

        void UseRunningAnimation();
        void UseIdleAnimation();

        void CreateIris(Scene* pScene);

        enum IrisAnimation {
            None,
            Idle,
            Run
        };

        IrisAnimation m_CurrentAnimation = None;
        float m_LastTimePositionRegistered = 0.0f;
        glm::vec3 m_LastPosition;
        glm::vec3 m_FollowedPosition;
        glm::vec2 m_Direction;
        std::queue<glm::vec3> m_CaliPositionsRegistered;
        uint32_t m_PositionRegisteredCount = 0;
        Transform3D* mp_Transform = nullptr;
        AnimationComponent* mp_AnimationComponent = nullptr;
    };
} // GPC

