#pragma once

#include "Stats.h"

namespace GPC {
    using Tier = uint8_t;
    using EquipmentID = uint32_t;
    constexpr Tier MIN_TIER = 1;
    constexpr Tier MAX_TIER = 5;

    enum class OwnerCharacter : uint8_t
    {
        Shared = 0,
        Cali   = 1,
        Iris   = 2
    };

    using CreateModifierFunc =  Modifier* (*)(Statistic*, float);

    struct Trait {
        const char* Name                            = nullptr;
        Tier TraitTier                              = MIN_TIER;
        float BaseValue                             = 0.0f;
        Modifier* pModifier                         = nullptr;
        CreateModifierFunc pRecreateModifierFunc    = nullptr;
    };

    struct BaseStat {
        const char* Name                            = nullptr;
        float Value                                 = 0.0f;
        Modifier* pModifier                         = nullptr;
        CreateModifierFunc pRecreateModifierFunc    = nullptr;
    };

    struct StatRollDescriptor {
        const char* Name;
        float Weight;
        glm::vec2 RangeTier1;
        glm::vec2 RangeTier2;
        glm::vec2 RangeTier3;
        glm::vec2 RangeTier4;
        CreateModifierFunc pCreateModifierFunc;
    };

    template<unsigned STAT_COUNT>
    struct TraitRollDescriptor {
        const char*     NAMES[STAT_COUNT];
        float     WEIGHTS[STAT_COUNT];
        glm::vec2 RANGES[STAT_COUNT][4];
        CreateModifierFunc FUNCTIONS[STAT_COUNT];
        constexpr TraitRollDescriptor(const StatRollDescriptor stat_descriptors[STAT_COUNT]);
    };

    template<unsigned STAT_COUNT>
    constexpr TraitRollDescriptor<STAT_COUNT>::TraitRollDescriptor(const StatRollDescriptor stat_descriptors[STAT_COUNT]) : NAMES{}, WEIGHTS{}, RANGES{} {
        for (uint32_t i = 0; i < STAT_COUNT; ++i) {
            NAMES[i] = stat_descriptors[i].Name;
            WEIGHTS[i] = stat_descriptors[i].Weight;
            RANGES[i][0] = stat_descriptors[i].RangeTier1;
            RANGES[i][1] = stat_descriptors[i].RangeTier2;
            RANGES[i][2] = stat_descriptors[i].RangeTier3;
            RANGES[i][3] = stat_descriptors[i].RangeTier4;
            FUNCTIONS[i] = stat_descriptors[i].pCreateModifierFunc;
        }
    }

    class Equipment {
        static inline EquipmentID NextEquipmentId   = 0;
        EquipmentID m_ID                            = 0;

    protected:
        StatisticGroup* mp_StatGroup                = nullptr;
        const Tier m_EquipmentTier                  = MIN_TIER;
        std::array<Trait, MAX_TIER - 1> m_Traits    = {};
        int8_t m_GoldTrait                          = -1;
        std::vector<BaseStat> m_BaseStatistic      = {};

        bool CanReceiveGoldTrait() const { return m_GoldTrait == -1; }

        void ReGenerateModifier();

        Tier GetRandomTierForTrait() const;

        static Modifier* CreateModifierAdd(Statistic* stat_name, float base);
        static Modifier* CreateModifierMultiply(Statistic* stat_name, float base);
        static Modifier* CreateModifierAddFinal(Statistic* stat_name, float base);

    public:
        virtual Trait CreateTraits(uint8_t index, bool can_be_t5) = 0;
        virtual void OnBaseStatistics() = 0;

        static constexpr float TierWeightsByEquipmentTier[MAX_TIER][MAX_TIER] {
            // Equipment Tier 1
            {
                50.0f, // Tier 1 (%)
                33.0f, // Tier 2 (%)
                15.0f, // Tier 3 (%)
                 2.0f, // Tier 4 (%)
                 0.0f  // Tier 5 (%)
            },
            // Equipment Tier 2
            {
                45.0f, // Tier 1 (%)
                30.0f, // Tier 2 (%)
                20.0f, // Tier 3 (%)
                 5.0f, // Tier 4 (%)
                 0.0f  // Tier 5 (%)
            },
            // Equipment Tier 3
            {
                38.0f, // Tier 1 (%)
                28.0f, // Tier 2 (%)
                23.5f, // Tier 3 (%)
                 9.5f, // Tier 4 (%)
                 1.0f  // Tier 5 (%)
            },
            // Equipment Tier 4
            {
                25.0f, // Tier 1 (%)
                25.0f, // Tier 2 (%)
                33.0f, // Tier 3 (%)
                15.0f, // Tier 4 (%)
                 2.0f  // Tier 5 (%)
            },
            // Equipment Tier 5
            {
                20.0f, // Tier 1 (%)
                20.0f, // Tier 2 (%)
                40.0f, // Tier 3 (%)
                17.5f, // Tier 4 (%)
                 2.5f  // Tier 5 (%)
            },
        };

        OwnerCharacter Owner = OwnerCharacter::Shared;

        virtual ~Equipment() = default;
        Equipment(Tier tier) : m_EquipmentTier(tier), m_ID(NextEquipmentId++) { };

        void ReRollTraits(std::bitset<MAX_TIER> to_reroll = 0b11111111);
        void EquipTo(StatisticGroup* pStatistiqueGrp);

        [[nodiscard]] bool IsEquipped() const { return mp_StatGroup != nullptr; }
        [[nodiscard]] uint8_t GetTraitCount() const { return m_EquipmentTier < MAX_TIER - 1 ? m_EquipmentTier : MAX_TIER - 1; }
        [[nodiscard]] const Trait& GetTrait(uint8_t index) const { return m_Traits[index]; }
        [[nodiscard]] StatisticGroup& GetStatGroup() const { return *mp_StatGroup; }
        [[nodiscard]] EquipmentID GetId() const { return m_ID; };

        [[nodiscard]] const std::array<Trait, MAX_TIER - 1>& GetTraits() const { return m_Traits; }
        [[nodiscard]] const std::vector<BaseStat>& GetBaseStatistics() const { return m_BaseStatistic; }
    };


    struct ArmorPiece : public Equipment {
        ArmorPiece(Tier tier) : Equipment(tier) {};
    };

    struct Helmet : public ArmorPiece {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        Helmet(Tier tier) : ArmorPiece(tier) {};
    };

    struct ChestPlate : public ArmorPiece {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        ChestPlate(Tier tier) : ArmorPiece(tier) {};
    };

    struct Gloves : public ArmorPiece {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        Gloves(Tier tier) : ArmorPiece(tier) {};
    };

    struct Legs : public ArmorPiece {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        Legs(Tier tier) : ArmorPiece(tier) {};
    };

    struct Boots : public ArmorPiece {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        Boots(Tier tier) : ArmorPiece(tier) {};
    };

    struct Weapon : public Equipment {
        Weapon(Tier tier) : Equipment(tier) {};
        virtual bool IsMelee() = 0;
    };

    struct MeleeWeapon : public Weapon {
        MeleeWeapon(Tier tier) : Weapon(tier) { }
        bool IsMelee() override { return true; }
    };

    struct DistanceWeapon : public Weapon {
        DistanceWeapon(Tier tier) : Weapon(tier) { }
        bool IsMelee() override { return false; }
    };

    struct ShortSword : public MeleeWeapon {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        ShortSword(Tier tier) : MeleeWeapon(tier) {};
    };

    struct LongSword : public MeleeWeapon {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        LongSword(Tier tier) : MeleeWeapon(tier) {};
    };

    struct Gauntlet : public MeleeWeapon {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        Gauntlet(Tier tier) : MeleeWeapon(tier) { Owner = OwnerCharacter::Cali; };
    };

    struct Axe : public MeleeWeapon {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        Axe(Tier tier) : MeleeWeapon(tier) { Owner = OwnerCharacter::Iris; };
    };

    struct Tomahawk : public DistanceWeapon {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        Tomahawk(Tier tier) : DistanceWeapon(tier) {};
    };

    struct Bow : public DistanceWeapon {
        Trait CreateTraits(uint8_t index, bool can_be_t5) override;
        void OnBaseStatistics() override;
        Bow(Tier tier) : DistanceWeapon(tier) {};
    };

} // GPC