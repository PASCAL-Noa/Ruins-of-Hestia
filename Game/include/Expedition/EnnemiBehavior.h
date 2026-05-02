#pragma once

#include "Behavior.h"
#include "Map Generation/EnnemiGeneratorBehavior.h"
#include "Expedition/PlayerInteraction.h"

namespace GPC {
    struct EnnemiBehavior : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(EnnemiBehavior);

        EnnemiGeneratorBehavior* pManager                       = nullptr;
        EnnemiGeneratorBehavior::EnnemiComponents* pComponents  = nullptr;
        Transform3D* pPlayer                                    = nullptr;
        LivingEntityBehavior* mp_Living                         = nullptr;

        void OnUpdate(const BehaviorUpdateContext *pCtx) override;
        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnDestroy() override;

    private:

        enum Phase {
            Standing,
            Roaming,
            Targeting
        };

        Transform3D* mp_Transform = nullptr;
        Render3DComponent* mp_Renderer = nullptr;

        ShortSword* mp_Weapon = nullptr;

        Phase m_CurrentPhase = Standing;
        bool m_IsAttacking = false;
        bool m_IsInDelayForAttack = false;
        float m_EndOfAttack = 0.0f;
        float m_StartOfAttack = 0.0f;

        glm::vec3 m_RoamingDirection = glm::vec3(0.0f, 0.0f, 0.0f);
        Transform3D* mp_Target = nullptr;
        LivingEntityBehavior* mp_LivingTarget = nullptr;
        float m_TimeStartPhase = 0.0f;
        float m_ExpectedDuration = 0.0f;

        float m_TimeLastAcquisition = 0.0f;

        void GoToStanding(float time, Scene* pScene);
        void GoToRoaming(float time, glm::vec3 direction, Scene* pScene);
        void GoToTargeting(float time, Transform3D* pTarget, Scene* pScene);

        void UpdateStanding(const BehaviorUpdateContext *pCtx);
        void UpdateRoaming(const BehaviorUpdateContext *pCtx);
        void UpdateTargeting(const BehaviorUpdateContext *pCtx);

        void UpdateRotation(const Scene* pScene);

        static constexpr float DELAY_ATTACK = 0.4f;

        static constexpr float STANDING_DURATION_MAX = 1.5f;
        static constexpr float STANDING_DURATION_MIN = 0.5f;

        static constexpr float ROAMING_DURATION_MAX = 2.5f;
        static constexpr float ROAMING_DURATION_MIN = 1.5f;
        static constexpr float ROAMING_WALL_DISTANCE_FOR_BREAK = 0.6f * GENERAL_SCALE_EXPEDITION;

        static constexpr float TIME_BETWEEN_TARGET_ACQUISITION_ROAMING = 0.5f;
        static constexpr float TIME_BETWEEN_TARGET_ACQUISITION_STANDING = 0.25f;

        static constexpr float TARGETING_DURATION_MAX = 5.0f;
        static constexpr float AUTO_RETARGET_DISTANCE_TARGETING_END = 3.0f * GENERAL_SCALE_EXPEDITION;

        static constexpr float TARGETING_DISTANCE_MAX_ROAMING = 2.25f * GENERAL_SCALE_EXPEDITION;
        static constexpr float TARGETING_DISTANCE_MAX_STANDING = 3.75f * GENERAL_SCALE_EXPEDITION;

        static constexpr float ROAMING_SPEED = 1.2f * GENERAL_SCALE_EXPEDITION;
        static constexpr float DEFAULT_ATTACK_RANGE = 0.85f * GENERAL_SCALE_EXPEDITION;
        static constexpr float TARGETING_MOVE_SPEED = 1.5f * GENERAL_SCALE_EXPEDITION;

        // static constexpr float AUTO_TARGETING_DISTANCE = 0.5f;
    };
} // GPC