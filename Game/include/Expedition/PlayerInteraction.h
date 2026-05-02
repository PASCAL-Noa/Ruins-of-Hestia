#pragma once

#include "Behavior.h"
#include "Map Generation/EnnemiGeneratorBehavior.h"
#include "Map Generation/ResourceGeneratorBehavior.h"
#include "RandomAudio.h"

namespace GPC {
    class AudioSource;
    struct PlayerInteraction : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(PlayerInteraction);

        EnnemiGeneratorBehavior* pEnnemiManager     = nullptr;
        ResourceGeneratorBehavior* pResourceManager = nullptr;
        LivingEntityBehavior* pLiving               = nullptr;
        RandomAudio* pMiningStonePlayer       = nullptr;
        RandomAudio* pMiningWoodPlayer        = nullptr;
        RandomAudio* pAttackPlayer            = nullptr;
        RandomAudio* pDamageEnemie            = nullptr;
        bool m_UseTuto                              = false;
        float m_StartTime                            = 10000.0f;

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        static constexpr float RANGE_RESCALE_FACTOR = 0.05f * GENERAL_SCALE_EXPEDITION;
        static void CreateHitBox(
            Scene* pScene,
            LivingEntityBehavior* pLiving,
            float time_start,
            glm::vec3 position,
            float range,
            float duration,
            bool target_player,
            const std::string& texture
            );
    private:
        float m_TimeAttackShouldEnd = 0;
        bool m_IsAttacking = false;
        bool m_HarvestTutoHasProc = false;
        bool m_CombatTutoHasProc = false;

        static constexpr float MAX_RESOURCE_DISTANCE_FOR_COLLECT = 1.5f * GENERAL_SCALE_EXPEDITION;

        static constexpr glm::vec3 ENNEMI_INDICATOR_OFFSET = glm::vec3{ 0.0f, 0.6f, 0.0f } * GENERAL_SCALE_EXPEDITION;

        float GetMeleeRange() const;

        void AttackEnnemiMelee(EnnemiGeneratorBehavior::EnnemiComponents* pEnnemi, Scene* pScene, float time);
        void CollectResource(ResourceGeneratorBehavior::ResourceComponents* pResource, Scene* pScene, float time);

        void CreateTargetIndicator(Scene* pScene);

        Transform3D* mp_Transform = nullptr;

        Transform3D* mp_CurrentTargetIndicatorTransform = nullptr;
        Render3DComponent* mp_CurrentTargetIndicatorRenderer = nullptr;

        Transform3D* mp_EmitterWoodTransform;
        ParticleEmitter* mp_EmitterWood;
        Transform3D* mp_EmitterStoneTransform;
        ParticleEmitter* mp_EmitterStone;
        Transform3D* mp_EmitterBlueGemTransform;
        ParticleEmitter* mp_EmitterBlueGem;
        Transform3D* mp_EmitterRedGemTransform;
        ParticleEmitter* mp_EmitterRedGem;
        Transform3D* mp_EmitterYellowGemTransform;
        ParticleEmitter* mp_EmitterYellowGem;
        Transform3D* mp_EmitterOrangeGemTransform;
        ParticleEmitter* mp_EmitterOrangeGem;
        Transform3D* mp_EmitterGreenGemTransform;
        ParticleEmitter* mp_EmitterGreenGem;
        Transform3D* mp_EmitterPurpleGemTransform;
        ParticleEmitter* mp_EmitterPurpleGem;

        // Audio
        AudioSource* mp_UIpopSfx = nullptr;
    };
} // GPC