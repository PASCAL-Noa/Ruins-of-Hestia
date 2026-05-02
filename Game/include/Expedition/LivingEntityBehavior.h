#pragma once

#include "Behavior.h"
#include "Equipment.h"
#include "Stats.h"
#include "Map Generation/EnnemiGeneratorBehavior.h"

namespace GPC {

    struct LivingEntityBehavior : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(LivingEntityBehavior);

        StatisticGroup EntityStats;
        StatisticGroup MeleeWeaponStats;
        StatisticGroup DistanceWeaponStats;

        Statistic* pMaxHealth;
        Statistic* pArmor;
        Statistic* pStrength;
        Statistic* pSpeed;
        Statistic* pResourceMulti;
        Statistic* pProvocation;

        Statistic* pMeleeDamage;
        Statistic* pMeleeCritChance;
        Statistic* pMeleeCritDamage;
        Statistic* pMeleeAttackSpeed;
        Statistic* pMeleeRange;

        Statistic* pDistanceDamage;
        Statistic* pDistanceCritChance;
        Statistic* pDistanceCritDamage;
        Statistic* pDistanceAttackSpeed;
        Statistic* pDistanceProjectileSpeed;
        Statistic* pDistancePenetration;
        Statistic* pDistanceProjectileSize;
        bool IsUsingMelee = true;

        void* pData;
        bool IsInit = false;
        bool OwnEquipment = false;

        float CurrentHP = 1.0f;

        // void(killed, scene)
        GPC::Function<void, void*, Scene*>* pDeathCallBack;
        // void(damaged, damaging, is_melee, scene)
        GPC::Function<void, LivingEntityBehavior*, LivingEntityBehavior*, bool, Scene*>* pTakeDamageCallBack;

        inline static EnnemiGeneratorBehavior* pEnnemiManager = nullptr;

        static void DeathKillAsEnnemi(void* pResource, Scene* pScene);
        static void DeathKillAsBoss(void* pResource, Scene* pScene);
        static void DeathGameOver(void* pResource, Scene* pScene);

        static void TakeDamageDefault(LivingEntityBehavior* pDamaged, LivingEntityBehavior* pDamaging, bool is_melee, Scene* pScene);

        void Attack(LivingEntityBehavior* pDamaging, bool is_melee, Scene* pScene);

        void InitAsCali();
        void InitAsEnnemi(uint32_t difficulty);
        void InitAsBoss(uint32_t difficulty);

        void EquipRandomEquipment(uint32_t mission_difficulty);

        bool HasHelmet() const { return mp_Helmet != nullptr; }
        Helmet* GetHelmet() const { return mp_Helmet;}
        void EquipHelmet(Helmet* pHelmet);
        void DeEquipHelmet();

        bool HasGloves() const { return mp_Gloves != nullptr; }
        Gloves* GetGloves() const { return mp_Gloves;}
        void EquipGloves(Gloves* pGloves);
        void DeEquipGloves();

        bool HasChestPlate() const { return mp_ChestPlate != nullptr; }
        ChestPlate* GetChestPlate() const {return mp_ChestPlate;}
        void EquipChestPlate(ChestPlate* pChestPlate);
        void DeEquipChestPlate();

        bool HasLegs() const { return mp_Legs != nullptr; }
        Legs* GetLegs() const { return mp_Legs;}
        void EquipLegs(Legs* pLegs);
        void DeEquipLegs();

        bool HasBoots() const { return mp_Boots != nullptr; }
        Boots* GetBoots() const { return mp_Boots;}
        void EquipBoots(Boots* pBoots);
        void DeEquipBoots();

        uint32_t CountArmorPieces() const;

        bool HasMelee() const { return mp_Melee != nullptr; }
        MeleeWeapon* GetMelee() const { return mp_Melee; }
        void EquipMelee(MeleeWeapon* pMelee);
        void DeEquipMelee();

        bool HasDistance() const { return mp_Distance != nullptr; }
        DistanceWeapon* GetDistance() const { return mp_Distance; }
        void EquipDistance(DistanceWeapon* pDistance);
        void DeEquipDistance();

        void OnLateUpdate(const BehaviorUpdateContext *pCtx) override;
        void OnDestroy() override;

    private:
        static void EquipEquipment(Equipment*& pSlot, StatisticGroup& stat_group, Equipment* pToEquip);
        static void DeEquipEquipment(Equipment*& pSlot);

        Helmet* mp_Helmet             = nullptr;
        Gloves* mp_Gloves             = nullptr;
        ChestPlate* mp_ChestPlate     = nullptr;
        Legs* mp_Legs                 = nullptr;
        Boots* mp_Boots               = nullptr;

        MeleeWeapon* mp_Melee         = nullptr;
        DistanceWeapon* mp_Distance   = nullptr;
    };

} // GPC
