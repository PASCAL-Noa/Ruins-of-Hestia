#include "Expedition/PlayerInteraction.h"

#include "Scene.h"
#include "Expedition/HitBox.h"
#include "Expedition/LivingEntityBehavior.h"
#include "Expedition/ResourceBehavior.h"
#include "Expedition/TutoBehaviors.h"
#include "AudioSystem.h"

namespace GPC {
    void PlayerInteraction::OnCreate(const BehaviorCreateContext *pCtx) {
        Behavior::OnCreate(pCtx);

        CreateTargetIndicator(pCtx->pScene);

        mp_Transform = pCtx->pScene->GetComponent<Transform3D>(GetEntityID());

        mp_UIpopSfx = pCtx->pScene->AddComponent<AudioSource>(pCtx->pScene->CreateEntityAs2D());
        mp_UIpopSfx->Path = "Resources/Audio/SFX/sfx_ui4.wav";
        mp_UIpopSfx->Resource = GPC_AUDIO("UI4");
        mp_UIpopSfx->TypeVoice = AudioType::SFX;
        mp_UIpopSfx->Volume = 1.0f;
        mp_UIpopSfx->Looping = false;

        auto emitter_wood = pCtx->pScene->CreateEntityAs3D();
        mp_EmitterWoodTransform = pCtx->pScene->GetComponent<Transform3D>(emitter_wood);
        mp_EmitterWood = pCtx->pScene->AddComponent<ParticleEmitter>(emitter_wood);
        mp_EmitterWood->pGeometry = GPC_MESH("Sphere");
        mp_EmitterWood->pComputeProgram = &GPC_COMPUTE_PROGRAM("Wood Particle Program")->Program;
        mp_EmitterWood->SetLooping(false);
        mp_EmitterWood->SetDuration(ResourceBehavior::COOLDOWN_COLLECT_DURATION * 0.95f, 0.0f);

        auto emitter_stone = pCtx->pScene->CreateEntityAs3D();
        mp_EmitterStoneTransform = pCtx->pScene->GetComponent<Transform3D>(emitter_stone);
        mp_EmitterStone = pCtx->pScene->AddComponent<ParticleEmitter>(emitter_stone);
        mp_EmitterStone->pGeometry = GPC_MESH("Sphere");
        mp_EmitterStone->pComputeProgram = &GPC_COMPUTE_PROGRAM("Stone Particle Program")->Program;
        mp_EmitterStone->SetLooping(false);
        mp_EmitterStone->SetDuration(ResourceBehavior::COOLDOWN_COLLECT_DURATION * 0.95f, 0.0f);

        auto emitter_blue_gem = pCtx->pScene->CreateEntityAs3D();
        mp_EmitterBlueGemTransform = pCtx->pScene->GetComponent<Transform3D>(emitter_blue_gem);
        mp_EmitterBlueGem = pCtx->pScene->AddComponent<ParticleEmitter>(emitter_blue_gem);
        mp_EmitterBlueGem->pGeometry = GPC_MESH("Sphere");
        mp_EmitterBlueGem->pComputeProgram = &GPC_COMPUTE_PROGRAM("BlueGem Particle Program")->Program;
        mp_EmitterBlueGem->SetLooping(false);
        mp_EmitterBlueGem->SetDuration(ResourceBehavior::COOLDOWN_COLLECT_DURATION * 0.95f, 0.0f);

        auto emitter_red_gem = pCtx->pScene->CreateEntityAs3D();
        mp_EmitterRedGemTransform = pCtx->pScene->GetComponent<Transform3D>(emitter_red_gem);
        mp_EmitterRedGem = pCtx->pScene->AddComponent<ParticleEmitter>(emitter_red_gem);
        mp_EmitterRedGem->pGeometry = GPC_MESH("Sphere");
        mp_EmitterRedGem->pComputeProgram = &GPC_COMPUTE_PROGRAM("RedGem Particle Program")->Program;
        mp_EmitterRedGem->SetLooping(false);
        mp_EmitterRedGem->SetDuration(ResourceBehavior::COOLDOWN_COLLECT_DURATION * 0.95f, 0.0f);

        auto emitter_yellow_gem = pCtx->pScene->CreateEntityAs3D();
        mp_EmitterYellowGemTransform = pCtx->pScene->GetComponent<Transform3D>(emitter_yellow_gem);
        mp_EmitterYellowGem = pCtx->pScene->AddComponent<ParticleEmitter>(emitter_yellow_gem);
        mp_EmitterYellowGem->pGeometry = GPC_MESH("Sphere");
        mp_EmitterYellowGem->pComputeProgram = &GPC_COMPUTE_PROGRAM("YellowGem Particle Program")->Program;
        mp_EmitterYellowGem->SetLooping(false);
        mp_EmitterYellowGem->SetDuration(ResourceBehavior::COOLDOWN_COLLECT_DURATION * 0.95f, 0.0f);

        auto emitter_green_gem = pCtx->pScene->CreateEntityAs3D();
        mp_EmitterGreenGemTransform = pCtx->pScene->GetComponent<Transform3D>(emitter_green_gem);
        mp_EmitterGreenGem = pCtx->pScene->AddComponent<ParticleEmitter>(emitter_green_gem);
        mp_EmitterGreenGem->pGeometry = GPC_MESH("Sphere");
        mp_EmitterGreenGem->pComputeProgram = &GPC_COMPUTE_PROGRAM("GreenGem Particle Program")->Program;
        mp_EmitterGreenGem->SetLooping(false);
        mp_EmitterGreenGem->SetDuration(ResourceBehavior::COOLDOWN_COLLECT_DURATION * 0.95f, 0.0f);

        auto emitter_purple_gem = pCtx->pScene->CreateEntityAs3D();
        mp_EmitterPurpleGemTransform = pCtx->pScene->GetComponent<Transform3D>(emitter_purple_gem);
        mp_EmitterPurpleGem = pCtx->pScene->AddComponent<ParticleEmitter>(emitter_purple_gem);
        mp_EmitterPurpleGem->pGeometry = GPC_MESH("Sphere");
        mp_EmitterPurpleGem->pComputeProgram = &GPC_COMPUTE_PROGRAM("PurpleGem Particle Program")->Program;
        mp_EmitterPurpleGem->SetLooping(false);
        mp_EmitterPurpleGem->SetDuration(ResourceBehavior::COOLDOWN_COLLECT_DURATION * 0.95f, 0.0f);

        auto emitter_orange_gem = pCtx->pScene->CreateEntityAs3D();
        mp_EmitterOrangeGemTransform = pCtx->pScene->GetComponent<Transform3D>(emitter_orange_gem);
        mp_EmitterOrangeGem = pCtx->pScene->AddComponent<ParticleEmitter>(emitter_orange_gem);
        mp_EmitterOrangeGem->pGeometry = GPC_MESH("Sphere");
        mp_EmitterOrangeGem->pComputeProgram = &GPC_COMPUTE_PROGRAM("OrangeGem Particle Program")->Program;
        mp_EmitterOrangeGem->SetLooping(false);
        mp_EmitterOrangeGem->SetDuration(ResourceBehavior::COOLDOWN_COLLECT_DURATION * 0.95f, 0.0f);
    }

    void PlayerInteraction::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (m_IsAttacking && pCtx->pClock->GetTime() > m_TimeAttackShouldEnd) {
            m_IsAttacking = false;
        }

        float ennemi_distance;
        glm::vec2 ennemi_direction;
        auto* closest_ennemi = pEnnemiManager->GetClosestEnnemi(ennemi_distance, ennemi_direction);

        float resource_distance;
        glm::vec2 resource_direction;
        auto* closest_resource = pResourceManager->GetClosestResource(resource_distance, resource_direction);

        Transform3D* pTarget = nullptr;
        bool target_resource = false;

        if (closest_ennemi != nullptr) {
            float melee_range = GetMeleeRange();
            if (ennemi_distance <= melee_range) {
                target_resource = false;
                pTarget = closest_ennemi->pTransform;
                if (m_IsAttacking == false)
                    AttackEnnemiMelee(closest_ennemi, pCtx->pScene, pCtx->pClock->GetTime());
            } else {
                // TODO : Attack Ranged
            }
        }
        if (closest_resource != nullptr && pTarget == nullptr && resource_distance < MAX_RESOURCE_DISTANCE_FOR_COLLECT) {
            target_resource = true;
            pTarget = closest_resource->pTransform1;
            CollectResource(closest_resource, pCtx->pScene, pCtx->pClock->GetTime());
        }

        if (pTarget == nullptr) {
            // GPC_INFO << "No target available" << ENDL;
            mp_CurrentTargetIndicatorRenderer->IsEnable = false;
        } else {
            glm::vec3 position = pTarget->GetWorldPosition();
            if (target_resource) {
                position += glm::vec3(0.0f, closest_resource->pBehavior->GetIndicatorOffsetY() * GENERAL_SCALE_EXPEDITION, 0.0f) + pTarget->LocalTransform.GetUp() * 0.02f;
            } else {
                position += ENNEMI_INDICATOR_OFFSET + pTarget->LocalTransform.GetUp() * 0.02f;
            }
            mp_CurrentTargetIndicatorTransform->LocalTransform.SetPosition(position);
            mp_CurrentTargetIndicatorRenderer->IsEnable = true;
        }
    }

    void PlayerInteraction::CreateHitBox(
        Scene *pScene,
        LivingEntityBehavior *pLiving,
        float time_start,
        glm::vec3 position,
        float range,
        float duration,
        bool target_player,
        const std::string &texture)
    {
        auto hitbox = pScene->CreateEntityAs3D();

        auto Transform = pScene->GetComponent<Transform3D>(hitbox);
        Transform->LocalTransform.SetPosition(position);
        Transform->LocalTransform.SetScale(range, range, range);

        auto Collider = pScene->AddComponent<Collision3DComponent>(hitbox);
        Collider->IsSphere = true;
        Collider->IsTrigger = true;
        Collider->IsKinematic = true;
        Collider->CollType = ColliderType::AllyProjectile;

        auto Debug = pScene->AddComponent<Render3DComponent>(hitbox);
        Debug->pGeometry = GPC_MESH("Sphere");
        Debug->SetTexture("1024x1024", texture);

        auto HitBox = pScene->AddBehavior<GPC::HitBox>(hitbox);
        HitBox->Descriptor.DestroyOnHit = false;
        HitBox->Descriptor.Duration = duration;
        HitBox->Descriptor.TargetPlayer = target_player;
        HitBox->Descriptor.pSourceLiving = pLiving;
        HitBox->Descriptor.StartTime = time_start;
        HitBox->Descriptor.IsMelee = true;
    }

    float PlayerInteraction::GetMeleeRange() const {
        return GENERAL_SCALE_EXPEDITION + pLiving->pMeleeRange->GetFinalValue() * RANGE_RESCALE_FACTOR;
    }

    void PlayerInteraction::AttackEnnemiMelee(EnnemiGeneratorBehavior::EnnemiComponents *pEnnemi, Scene* pScene, float time) {
        float atk_speed = pLiving->pMeleeAttackSpeed->GetFinalValue();
        float cooldown = 1.0f - atk_speed * 0.05f;
        if (cooldown <= 0.0f) cooldown = 0.0f;

        m_IsAttacking = true;
        m_TimeAttackShouldEnd = time + cooldown;

        glm::vec3 diff =  pEnnemi->pTransform->GetWorldPosition() - mp_Transform->GetWorldPosition();
        glm::vec3 direction = glm::normalize(diff);

        float range = GetMeleeRange();
        glm::vec3 position = mp_Transform->GetWorldPosition() + direction * range * 0.5f;
        CreateHitBox(
            pScene,
            pLiving,
            time,
            position,
            range,
            0.1f,
            false,
            "Blue"
        );

        if (m_CombatTutoHasProc == false && m_UseTuto && time > m_StartTime + 3.1f
            && TutoQueue::CanSpawnTuto(static_cast<float>(time))) {
            mp_UIpopSfx->Play();
            auto tuto1 = pScene->CreateEntityAs3D();
            pScene->AddBehavior<ExpeditionCombatTuto>(tuto1);
            m_CombatTutoHasProc = true;
        }

        if (pAttackPlayer) pAttackPlayer->Play();
        if (pDamageEnemie) pDamageEnemie->Play();
    }

    void PlayerInteraction::CollectResource(ResourceGeneratorBehavior::ResourceComponents *pResource, Scene* pScene, float time) {
        if (pResource->pBehavior->TryCollect(pResource, pScene, time)) {

            switch (pResource->pBehavior->m_ResourceType) {
                case ResourceType::WOOD:
                    if (pMiningWoodPlayer) pMiningWoodPlayer->Play();
                    mp_EmitterWoodTransform->LocalTransform.SetPosition(pResource->pTransform1->GetWorldPosition());
                    mp_EmitterWood->Play();
                    break;
                default:
                case ResourceType::STONE:
                    if (pMiningStonePlayer) pMiningStonePlayer->Play();
                    mp_EmitterStoneTransform->LocalTransform.SetPosition(pResource->pTransform1->GetWorldPosition() + glm::vec3(0, 1, 0));
                    mp_EmitterStone->Play();
                    break;
                case ResourceType::GREEN_GEMME:
                    if (pMiningStonePlayer) pMiningStonePlayer->Play();
                    mp_EmitterGreenGemTransform->LocalTransform.SetPosition(pResource->pTransform1->GetWorldPosition() + glm::vec3(0, 1, 0));
                    mp_EmitterGreenGem->Play();
                    break;
                case ResourceType::YELLOW_GEMME:
                    if (pMiningStonePlayer) pMiningStonePlayer->Play();
                    mp_EmitterYellowGemTransform->LocalTransform.SetPosition(pResource->pTransform1->GetWorldPosition() + glm::vec3(0, 1, 0));
                    mp_EmitterYellowGem->Play();
                    break;
                case ResourceType::RED_GEMME:
                    if (pMiningStonePlayer) pMiningStonePlayer->Play();
                    mp_EmitterRedGemTransform->LocalTransform.SetPosition(pResource->pTransform1->GetWorldPosition() + glm::vec3(0, 1, 0));
                    mp_EmitterRedGem->Play();
                    break;
                case ResourceType::BLUE_GEMME:
                    if (pMiningStonePlayer) pMiningStonePlayer->Play();
                    mp_EmitterBlueGemTransform->LocalTransform.SetPosition(pResource->pTransform1->GetWorldPosition() + glm::vec3(0, 1, 0));
                    mp_EmitterBlueGem->Play();
                    break;
                case ResourceType::ORANGE_GEMME:
                    if (pMiningStonePlayer) pMiningStonePlayer->Play();
                    mp_EmitterOrangeGemTransform->LocalTransform.SetPosition(pResource->pTransform1->GetWorldPosition() + glm::vec3(0, 1, 0));
                    mp_EmitterOrangeGem->Play();
                    break;
                case ResourceType::PURPLE_GEMME:
                    if (pMiningStonePlayer) pMiningStonePlayer->Play();
                    mp_EmitterPurpleGemTransform->LocalTransform.SetPosition(pResource->pTransform1->GetWorldPosition() + glm::vec3(0, 1, 0));
                    mp_EmitterPurpleGem->Play();
                    break;
            }

            if (m_HarvestTutoHasProc == false && m_UseTuto && time > m_StartTime + 3.1f
                && TutoQueue::CanSpawnTuto(static_cast<float>(time))) {
                mp_UIpopSfx->Play();
                auto tuto1 = pScene->CreateEntityAs3D();
                pScene->AddBehavior<ExpeditionHarvestTuto>(tuto1);
                m_HarvestTutoHasProc = true;
            }

            GPC_INFO << "Collecting Resource : " << pResource->pTransform1->GetEntityID() << " !" << ENDL;
        }
    }

    void PlayerInteraction::CreateTargetIndicator(Scene *pScene) {
        auto target = pScene->CreateEntityAs3D();

        mp_CurrentTargetIndicatorTransform = pScene->GetComponent<Transform3D>(target);
        mp_CurrentTargetIndicatorRenderer = pScene->AddComponent<Render3DComponent>(target);

        mp_CurrentTargetIndicatorTransform->LocalTransform.SetScale(0.75f * GENERAL_SCALE_EXPEDITION, 1.0f, 1.05f * GENERAL_SCALE_EXPEDITION);
        mp_CurrentTargetIndicatorTransform->LocalTransform.SetRotationYawPitchRoll(GPC_PI * 0.25f, GPC_PI * 0.5f, GPC_PI);

        mp_CurrentTargetIndicatorRenderer->IsEnable = false;
        mp_CurrentTargetIndicatorRenderer->pGeometry = GPC_MESH("Plane");
        mp_CurrentTargetIndicatorRenderer->SetTexture("619x642", "Fleche");
    }
} // GPC