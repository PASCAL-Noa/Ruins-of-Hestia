#include "Expedition/EnnemiBehavior.h"

#include "Random.h"
#include "Scene.h"
#include "StateMachine_PlantTest.h"
#include "Expedition/LivingEntityBehavior.h"
#include "Expedition/PlayerController.h"

namespace GPC {
    void EnnemiBehavior::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (pComponents->pLiving == nullptr) return;

        switch (m_CurrentPhase) {
            default:
            case Phase::Standing: UpdateStanding(pCtx); break;
            case Phase::Roaming: UpdateRoaming(pCtx); break;
            case Phase::Targeting: UpdateTargeting(pCtx); break;
        }

    }

    void EnnemiBehavior::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);
        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());
        mp_Renderer = pCtx->pScene->GetComponent<Render3DComponent>(GetEntityID());
        GoToStanding(pCtx->pClock->GetTime(), pCtx->pScene);
    }

    void EnnemiBehavior::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);
        pComponents->pLiving->EquipRandomEquipment(pManager->Difficulty);
        pComponents->pLiving->CurrentHP = pComponents->pLiving->pMaxHealth->GetFinalValue();

        glm::vec3 scale_add_foreach_armor_piece = glm::vec3(0.1f) * GENERAL_SCALE_EXPEDITION * pComponents->pLiving->CountArmorPieces();
        pComponents->pTransform->LocalTransform.AddScale(scale_add_foreach_armor_piece);
    }

    void EnnemiBehavior::OnDestroy() {
        Behavior::OnDestroy();
        // pComponents->pLiving->DeEquipMelee();
        // delete mp_Weapon;
    }

    void EnnemiBehavior::GoToStanding(float time, Scene* pScene) {
        m_CurrentPhase = Phase::Standing;
        m_TimeStartPhase = time;
        m_ExpectedDuration = GPC::Random::Float(STANDING_DURATION_MIN, STANDING_DURATION_MAX);
        m_TimeLastAcquisition = time;
        // mp_Renderer->SetTexture("2048x2048", "EnnemiTexture");
    }

    void EnnemiBehavior::GoToRoaming(float time, glm::vec3 direction, Scene* pScene) {
        m_CurrentPhase = Phase::Roaming;
        m_RoamingDirection = direction;
        m_TimeStartPhase = time;
        m_ExpectedDuration = GPC::Random::Float(ROAMING_DURATION_MIN, ROAMING_DURATION_MAX);
        m_TimeLastAcquisition = time;
        // mp_Renderer->SetTexture("2048x2048", "EnnemiTexture");
    }

    void EnnemiBehavior::GoToTargeting(float time, Transform3D *pTarget, Scene* pScene) {
        m_CurrentPhase = Phase::Targeting;
        mp_Target = pTarget;
        mp_LivingTarget = pScene->GetBehavior<LivingEntityBehavior>(pTarget->GetEntityID());
        m_TimeStartPhase = time;
        // mp_Renderer->SetTexture("2048x2048", "EnnemiTexture");
    }

    void EnnemiBehavior::UpdateStanding(const BehaviorUpdateContext *pCtx) {

        float time_since_last_acquisition = pCtx->pClock->GetTime() - m_TimeLastAcquisition;
        if (time_since_last_acquisition > TIME_BETWEEN_TARGET_ACQUISITION_STANDING) {
            m_TimeLastAcquisition = pCtx->pClock->GetTime();
            float distance_to_player = glm::distance(pPlayer->GetWorldPosition(), mp_Transform->GetWorldPosition());
            if (distance_to_player < TARGETING_DISTANCE_MAX_STANDING) {
                GoToTargeting(pCtx->pClock->GetTime(), pPlayer, pCtx->pScene);
                return;
            }
        }

        float duration = pCtx->pClock->GetTime() - m_TimeStartPhase;
        if (duration > m_ExpectedDuration) {
            float random = Random::Float(0.0f, GPC_PI * 2.0f);
            glm::vec3 Direction = { cos(random), 0.0f, sin(random) };
            GoToRoaming(pCtx->pClock->GetTime(), Direction, pCtx->pScene);
        }
    }

    void EnnemiBehavior::UpdateRoaming(const BehaviorUpdateContext *pCtx) {
        mp_Transform->LocalTransform.AddPosition(m_RoamingDirection * ROAMING_SPEED * pCtx->pClock->GetDeltaTime());
        UpdateRotation(pCtx->pScene);

        float time_since_last_acquisition = pCtx->pClock->GetTime() - m_TimeLastAcquisition;
        if (time_since_last_acquisition > TIME_BETWEEN_TARGET_ACQUISITION_ROAMING) {
            m_TimeLastAcquisition = pCtx->pClock->GetTime();
            float distance_to_player = glm::distance(pPlayer->GetWorldPosition(), mp_Transform->GetWorldPosition());
            if (distance_to_player < TARGETING_DISTANCE_MAX_ROAMING) {
                GoToTargeting(pCtx->pClock->GetTime(), pPlayer, pCtx->pScene);
                return;
            }
        }

        const Ray ray{mp_Transform->GetWorldPosition() + m_RoamingDirection * 0.303f * GENERAL_SCALE_EXPEDITION, m_RoamingDirection};
        RayHit hit;
        if (pCtx->pScene->RayCast3D(hit, ray)) {
            float distance_to_hit = glm::distance(hit.ContactPoint, mp_Transform->GetWorldPosition());
            if (distance_to_hit < ROAMING_WALL_DISTANCE_FOR_BREAK) {
                GoToStanding(pCtx->pClock->GetTime(), pCtx->pScene);
                return;
            }
        }

        float duration = pCtx->pClock->GetTime() - m_TimeStartPhase;
        if (duration > m_ExpectedDuration) {
            GoToStanding(pCtx->pClock->GetTime(), pCtx->pScene);
        }
    }

    void EnnemiBehavior::UpdateTargeting(const BehaviorUpdateContext *pCtx) {
        float atk_speed = mp_Living->pMeleeAttackSpeed->GetFinalValue();
        float atk_range_melee = DEFAULT_ATTACK_RANGE + mp_Living->pMeleeRange->GetFinalValue() * PlayerInteraction::RANGE_RESCALE_FACTOR;
        float cooldown = 1.0f - atk_speed * 0.05f;
        if (cooldown <= 0.0f) cooldown = 0.0f;

        if (m_IsInDelayForAttack) {
            if (pCtx->pClock->GetTime() >= m_StartOfAttack) {
                // CREATE HITBOX
                glm::vec3 diff = mp_Target->GetWorldPosition() - pComponents->pTransform->GetWorldPosition();
                diff.y = 0.0f;
                glm::vec3 direction = glm::normalize(diff);
                glm::vec3 position = pComponents->pTransform->GetWorldPosition() + direction * atk_range_melee * 0.5f;
                PlayerInteraction::CreateHitBox(
                    pCtx->pScene,
                    pComponents->pLiving,
                    pCtx->pClock->GetTime(),
                    position,
                    atk_range_melee,
                    0.1f,
                    true,
                    "Red"
                );
                m_IsInDelayForAttack = false;
                m_IsAttacking = true;
            }
            return;
        }

        if (m_IsAttacking) {
            if (pCtx->pClock->GetTime() >= m_EndOfAttack) {
                m_IsAttacking = false;
            }
            return;
        }

        float distance_to_player = glm::distance(pPlayer->GetWorldPosition(), mp_Transform->GetWorldPosition());
        if (distance_to_player < atk_range_melee * 0.98f) {
            m_IsInDelayForAttack = true;
            m_StartOfAttack = pCtx->pClock->GetTime() + DELAY_ATTACK;
            m_EndOfAttack = m_StartOfAttack + cooldown;
            return;
        }

        glm::vec2 toward = {
            pPlayer->GetWorldPosition().x - mp_Transform->GetWorldPosition().x,
            pPlayer->GetWorldPosition().z - mp_Transform->GetWorldPosition().z
        };
        toward = glm::normalize(toward);
        m_RoamingDirection = {toward.x, 0.0f, toward.y};
        glm::vec3 movement = glm::vec3{ toward.x, 0.0f, toward.y } * TARGETING_MOVE_SPEED * pCtx->pClock->GetDeltaTime();
        mp_Transform->LocalTransform.AddPosition(movement);
        UpdateRotation(pCtx->pScene);

        float duration = pCtx->pClock->GetTime() - m_TimeStartPhase;
        if (duration > TARGETING_DURATION_MAX) {
            if (distance_to_player < AUTO_RETARGET_DISTANCE_TARGETING_END) {
                GoToTargeting(pCtx->pClock->GetTime(), mp_Target, pCtx->pScene);
            }
            else {
                GoToStanding(pCtx->pClock->GetTime(), pCtx->pScene);
            }
        }
    }

    void EnnemiBehavior::UpdateRotation(const Scene *pScene) {
        mp_Transform->LocalTransform.SetRotationYawPitchRoll(
            -atan2( m_RoamingDirection.z, m_RoamingDirection.x ) + GPC_PI * 0.5f,
            0.0f,
            0.0f
        );
    }

} // GPC