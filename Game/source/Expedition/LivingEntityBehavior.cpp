#include "Expedition/LivingEntityBehavior.h"

#include "Random.h"
#include "Scene.h"
#include "Expedition/BossBarHandler.h"
#include "Scenes/SceneExpedition.h"
#include "Scenes/SceneVillageV1.h"

namespace GPC {

    void LivingEntityBehavior::DeathKillAsEnnemi(void * pResource, Scene *pScene) {
        EnnemiGeneratorBehavior::EnnemiComponents* pEnnemi = static_cast<EnnemiGeneratorBehavior::EnnemiComponents*>(pResource);
        pEnnemiManager->KillEnnemiByEID(pEnnemi->EID, pEnnemi->Room, pScene);
    }

    void LivingEntityBehavior::DeathKillAsBoss(void *pResource, Scene *pScene) {
        EnnemiGeneratorBehavior::EnnemiComponents* pEnnemi = static_cast<EnnemiGeneratorBehavior::EnnemiComponents*>(pResource);
        pEnnemiManager->KillEnnemiByEID(pEnnemi->EID, pEnnemi->Room, pScene);
        auto BBH = pScene->GetBehavior<BossBarHandler>(pEnnemi->EID);
        BBH->mp_Healh->IsEnable = false;
        BBH->mp_HealhBackGround->IsEnable = false;
    }


    void LivingEntityBehavior::DeathGameOver(void * pResource, Scene *pScene) {
        GPC_INFO << "GAME OVER !!!!" << ENDL;

        auto scene = pointer_cast<SceneExpedition*>(pScene);
        scene->GetCali()->CurrentHP = scene->GetCali()->pMaxHealth->GetFinalValue();

        Inventory* pInventory = scene->GetInventory();
        pInventory->LosePercent(0.7f);
        scene->ShowPostGameUI(scene->GetDifficulty(), false);

    }

    void LivingEntityBehavior::TakeDamageDefault(LivingEntityBehavior *pDamaged, LivingEntityBehavior *pDamaging, bool is_melee, Scene *pScene) {
        if (is_melee) {
            float RawDamage = pDamaging->pMeleeDamage->GetFinalValue() * pDamaging->pStrength->GetFinalValue();

            // critic
            float chance = pDamaging->pMeleeCritChance->GetFinalValue();
            int base_crit = static_cast<int>(chance);
            float left_over_crit = chance - base_crit;
            int crit = base_crit + GPC::Random::Float() <= left_over_crit ? 1 : 0;
            float crit_multiplier = glm::pow(pDamaging->pMeleeCritDamage->GetFinalValue(), crit);

            // dmg reduction
            float dmg_reduction = static_cast<float>(exp(-pDamaged->pArmor->GetFinalValue() * 0.01f));

            float final_dmg = glm::max(0.0f, RawDamage * crit_multiplier * dmg_reduction);
            GPC_INFO << pDamaging->GetEntityID() << " inflicted " << static_cast<int>(final_dmg) << "dmg to " << pDamaged->GetEntityID() << " (raw : " << static_cast<int>(RawDamage) << ", crit : x" << crit_multiplier << ", dmg reduction : x" << dmg_reduction << ") " << ENDL;
            pDamaged->CurrentHP -= final_dmg;
        }
        else {
            float RawDamage = pDamaging->pDistanceDamage->GetFinalValue() * pDamaging->pStrength->GetFinalValue();
            GPC_INFO << pDamaging->GetEntityID() << " inflicted " << RawDamage << "dmg to " << pDamaged->GetEntityID() << ENDL;
            pDamaged->CurrentHP -= RawDamage;
        }
    }

    void LivingEntityBehavior::Attack(LivingEntityBehavior *pDamaging, bool is_melee, Scene *pScene) {
        if (pTakeDamageCallBack == nullptr) return;
        pTakeDamageCallBack->Call(this, std::move(pDamaging), std::move(is_melee), std::move(pScene));
    }

    void LivingEntityBehavior::InitAsCali() {
        if (IsInit) return;

        // Character
        pMaxHealth = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_HEALTH_POINT                            , 150.0f);
        pArmor = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_ARMOR                                       , 10.0f);
        pStrength = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_STRENGTH                                 , 1.0f);
        pSpeed = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_SPEED                                       , 1.0f);
        pResourceMulti = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_RESOURCE_MULT                       , 1.0f);
        pProvocation = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_PROVOCATION                           , 0.0f);

        // Melee
        pMeleeDamage = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DAMAGE                                , 0.0f);
        pMeleeCritChance = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_CHANCE                       , 0.0f);
        pMeleeCritDamage = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_DMG                          , 0.0f);
        pMeleeAttackSpeed = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_ATTACK_SPEED                     , 1.0f);
        pMeleeRange = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_MELEE_RANGE                            , 0.0f);

        // Distance
        pDistanceDamage = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DAMAGE                             , 0.0f);
        pDistanceCritChance = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_CHANCE                    , 0.0f);
        pDistanceCritDamage = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_DMG                       , 0.0f);
        pDistanceAttackSpeed = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_ATTACK_SPEED                  , 1.0f);
        pDistanceProjectileSpeed = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DISTANCE_PROJECTILE_SPEED , 1.0f);
        pDistancePenetration = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DISTANCE_PENETRATION          , 0.0f);
        pDistanceProjectileSize = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DISTANCE_PROJECTILE_SIZE   , 1.0f);

        pDeathCallBack = MakeCallablePtr(DeathGameOver);
        pTakeDamageCallBack = MakeCallablePtr(TakeDamageDefault);

        CurrentHP = pMaxHealth->GetFinalValue();
        IsInit = true;
    }

    void LivingEntityBehavior::InitAsEnnemi(uint32_t difficulty) {
        if (IsInit) return;

        // Character
        pMaxHealth = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_HEALTH_POINT                            , 35.0f * difficulty);
        pArmor = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_ARMOR                                       , 0.0f);
        pStrength = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_STRENGTH                                 , 1.0f);
        pSpeed = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_SPEED                                       , 1.0f);
        pResourceMulti = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_RESOURCE_MULT                       , 1.0f);
        pProvocation = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_PROVOCATION                           , 0.0f);

        // Melee
        pMeleeDamage = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DAMAGE                                , 0.0f);
        pMeleeCritChance = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_CHANCE                       , 0.0f);
        pMeleeCritDamage = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_DMG                          , 0.0f);
        pMeleeAttackSpeed = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_ATTACK_SPEED                     , 1.0f);
        pMeleeRange = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_MELEE_RANGE                            , 0.0f);

        // Distance
        pDistanceDamage = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DAMAGE                             , 0.0f);
        pDistanceCritChance = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_CHANCE                    , 0.0f);
        pDistanceCritDamage = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_DMG                       , 0.0f);
        pDistanceAttackSpeed = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_ATTACK_SPEED                  , 1.0f);
        pDistanceProjectileSpeed = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DISTANCE_PROJECTILE_SPEED , 1.0f);
        pDistancePenetration = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DISTANCE_PENETRATION          , 0.0f);
        pDistanceProjectileSize = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DISTANCE_PROJECTILE_SIZE   , 1.0f);

        pDeathCallBack = MakeCallablePtr(DeathKillAsEnnemi);
        pTakeDamageCallBack = MakeCallablePtr(TakeDamageDefault);

        CurrentHP = pMaxHealth->GetFinalValue();
        IsInit = true;
    }

    void LivingEntityBehavior::InitAsBoss(uint32_t difficulty) {
        if (IsInit) return;

        // Character
        pMaxHealth = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_HEALTH_POINT                            , 200.0f * difficulty);
        pArmor = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_ARMOR                                       , 5.0f);
        pStrength = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_STRENGTH                                 , 2.0f);
        pSpeed = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_SPEED                                       , 1.0f);
        pResourceMulti = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_RESOURCE_MULT                       , 1.0f);
        pProvocation = &EntityStats.AddStat(GPC::Statistic::STAT_NAME_PROVOCATION                           , 0.0f);

        // Melee
        pMeleeDamage = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DAMAGE                                , 0.0f);
        pMeleeCritChance = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_CHANCE                       , 0.0f);
        pMeleeCritDamage = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_DMG                          , 0.0f);
        pMeleeAttackSpeed = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_ATTACK_SPEED                     , 1.0f);
        pMeleeRange = &MeleeWeaponStats.AddStat(GPC::Statistic::STAT_NAME_MELEE_RANGE                            , 0.0f);

        // Distance
        pDistanceDamage = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DAMAGE                             , 0.0f);
        pDistanceCritChance = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_CHANCE                    , 0.0f);
        pDistanceCritDamage = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_CRIT_DMG                       , 0.0f);
        pDistanceAttackSpeed = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_ATTACK_SPEED                  , 1.0f);
        pDistanceProjectileSpeed = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DISTANCE_PROJECTILE_SPEED , 1.0f);
        pDistancePenetration = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DISTANCE_PENETRATION          , 0.0f);
        pDistanceProjectileSize = &DistanceWeaponStats.AddStat(GPC::Statistic::STAT_NAME_DISTANCE_PROJECTILE_SIZE   , 1.0f);

        pDeathCallBack = MakeCallablePtr(DeathKillAsBoss);
        pTakeDamageCallBack = MakeCallablePtr(TakeDamageDefault);

        CurrentHP = pMaxHealth->GetFinalValue();
        IsInit = true;
    }

    void LivingEntityBehavior::EquipRandomEquipment(uint32_t mission_difficulty) {
        OwnEquipment = true;

        if (rand() % 2) {
            Helmet* helmet = new Helmet(mission_difficulty);
            helmet->OnBaseStatistics();
            helmet->ReRollTraits();
            EquipHelmet(helmet);
        }

        if (rand() % 2) {
            ChestPlate* chestPlate = new ChestPlate(mission_difficulty);
            chestPlate->OnBaseStatistics();
            chestPlate->ReRollTraits();
            EquipChestPlate(chestPlate);
        }

        if (rand() % 2) {
            Gloves* gloves = new Gloves(mission_difficulty);
            gloves->OnBaseStatistics();
            gloves->ReRollTraits();
            EquipGloves(gloves);
        }

        if (rand() % 2) {
            Legs* legs = new Legs(mission_difficulty);
            legs->OnBaseStatistics();
            legs->ReRollTraits();
            EquipLegs(legs);
        }

        if (rand() % 2) {
            Boots* boots = new Boots(mission_difficulty);
            boots->OnBaseStatistics();
            boots->ReRollTraits();
            EquipBoots(boots);
        }

        MeleeWeapon* melee;
        if (rand() % 2) {
            melee = new ShortSword(mission_difficulty);
        } else {
            melee = new LongSword(mission_difficulty);
        }
        melee->OnBaseStatistics();
        melee->ReRollTraits();
        EquipMelee(melee);

        // DistanceWeapon* distance;
        // if (rand() % 2) {
        //     distance = new Tomahawk(mission_difficulty);
        // } else {
        //     distance = new Bow(mission_difficulty);
        // }
        // distance->OnBaseStatistics();
        // distance->ReRollTraits();
        // EquipDistance(distance);
    }

    void LivingEntityBehavior::EquipHelmet(Helmet *pHelmet) {
        auto myHelmet = static_cast<Equipment*>(mp_Helmet);
        EquipEquipment(myHelmet, EntityStats, pHelmet);
    }

    void LivingEntityBehavior::DeEquipHelmet() {
        auto myHelmet = static_cast<Equipment*>(mp_Helmet);
        DeEquipEquipment(myHelmet);
    }

    void LivingEntityBehavior::EquipGloves(Gloves *pGloves) {
        auto myGloves = static_cast<Equipment*>(mp_Gloves);
        EquipEquipment(myGloves, EntityStats, pGloves);
    }

    void LivingEntityBehavior::DeEquipGloves() {
        auto myGloves = static_cast<Equipment*>(mp_Gloves);
        DeEquipEquipment(myGloves);
    }

    void LivingEntityBehavior::EquipChestPlate(ChestPlate *pChestPlate) {
        auto myChestPlate = static_cast<Equipment*>(mp_ChestPlate);
        EquipEquipment(myChestPlate, EntityStats, pChestPlate);
    }

    void LivingEntityBehavior::DeEquipChestPlate() {
        auto myChestPlate = static_cast<Equipment*>(mp_ChestPlate);
        DeEquipEquipment(myChestPlate);
    }

    void LivingEntityBehavior::EquipLegs(Legs *pLegs) {
        auto myLegs = static_cast<Equipment*>(mp_Legs);
        EquipEquipment(myLegs, EntityStats, pLegs);
    }

    void LivingEntityBehavior::DeEquipLegs() {
        auto myLegs = static_cast<Equipment*>(mp_Legs);
        DeEquipEquipment(myLegs);
    }

    void LivingEntityBehavior::EquipBoots(Boots *pBoots) {
        auto myBoots = static_cast<Equipment*>(mp_Boots);
        EquipEquipment(myBoots, EntityStats, pBoots);

    }

    void LivingEntityBehavior::DeEquipBoots() {
        auto myBoots = static_cast<Equipment*>(mp_Boots);
        DeEquipEquipment(myBoots);

    }

    uint32_t LivingEntityBehavior::CountArmorPieces() const {
        uint32_t count = 0;
        if (HasHelmet()) count++;
        if (HasGloves()) count++;
        if (HasChestPlate()) count++;
        if (HasLegs()) count++;
        if (HasBoots()) count++;
        return count;
    }

    void LivingEntityBehavior::EquipMelee(MeleeWeapon *pMelee) {
        auto myMeleeWeapon = static_cast<Equipment*>(mp_Melee);
        EquipEquipment(myMeleeWeapon, MeleeWeaponStats, pMelee);
    }

    void LivingEntityBehavior::DeEquipMelee() {
        auto myMeleeWeapon = static_cast<Equipment*>(mp_Melee);
        DeEquipEquipment(myMeleeWeapon);
    }

    void LivingEntityBehavior::EquipDistance(DistanceWeapon *pDistance) {
        auto myDistanceWeapon = static_cast<Equipment*>(mp_Distance);
        EquipEquipment(myDistanceWeapon, DistanceWeaponStats, pDistance);
    }

    void LivingEntityBehavior::DeEquipDistance() {
        auto myDistance = static_cast<Equipment*>(mp_Distance);
        DeEquipEquipment(myDistance);
    }

    void LivingEntityBehavior::OnLateUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnLateUpdate(pCtx);

        if (CurrentHP <= 0.0f) {
            if (pDeathCallBack == nullptr) {
                GPC_INFO << "No CallBack On Death" << ENDL;
                return;
            }
            pDeathCallBack->Call(const_cast<void*>(pData), const_cast<Scene*>(pCtx->pScene));
        }
    }

    void LivingEntityBehavior::OnDestroy() {
        Behavior::OnDestroy();

        delete pDeathCallBack;
        delete pTakeDamageCallBack;

        if (OwnEquipment) {
            if (mp_Helmet != nullptr) {
                DeEquipHelmet();
                delete mp_Helmet;
            }
            if (mp_ChestPlate != nullptr) {
                DeEquipChestPlate();
                delete mp_ChestPlate;
            }
            if (mp_Gloves != nullptr) {
                DeEquipGloves();
                delete mp_Gloves;
            }
            if (mp_Legs != nullptr) {
                DeEquipLegs();
                delete mp_Legs;
            }
            if (mp_Boots != nullptr) {
                DeEquipBoots();
                delete mp_Boots;
            }
            if (mp_Melee != nullptr) {
                DeEquipMelee();
                delete mp_Melee;
            }
            if (mp_Distance != nullptr) {
                DeEquipDistance();
                delete mp_Distance;
            }
        }

        pMaxHealth = nullptr;
        pArmor = nullptr;
        pStrength = nullptr;
        pSpeed = nullptr;
        pResourceMulti = nullptr;
        pProvocation = nullptr;

        pMeleeDamage = nullptr;
        pMeleeCritChance = nullptr;
        pMeleeCritDamage = nullptr;
        pMeleeAttackSpeed = nullptr;
        pMeleeRange = nullptr;

        pDistanceDamage = nullptr;
        pDistanceCritChance = nullptr;
        pDistanceCritDamage = nullptr;
        pDistanceAttackSpeed = nullptr;
        pDistanceProjectileSpeed = nullptr;
        pDistancePenetration = nullptr;
        pDistanceProjectileSize = nullptr;

        mp_Helmet = nullptr;
        mp_Gloves = nullptr;
        mp_ChestPlate = nullptr;
        mp_Legs = nullptr;
        mp_Boots = nullptr;

        mp_Melee = nullptr;
        mp_Distance = nullptr;

        pDeathCallBack = nullptr;
        pTakeDamageCallBack = nullptr;

        pData = nullptr;
    }

    void LivingEntityBehavior::EquipEquipment(Equipment *&pSlot, StatisticGroup& stat_group, Equipment *pToEquip) {
        if (pSlot != nullptr)
            pSlot->EquipTo(nullptr);
        pSlot = pToEquip;
        pSlot->EquipTo(&stat_group);
    }

    void LivingEntityBehavior::DeEquipEquipment(Equipment *&pSlot) {
        if (pSlot != nullptr)
            pSlot->EquipTo(nullptr);
        pSlot = nullptr;
    }
} // GPC