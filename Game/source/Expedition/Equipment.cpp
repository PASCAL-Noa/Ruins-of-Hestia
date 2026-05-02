#include "Expedition/Equipment.h"

#include "Random.h"

namespace GPC {

    void Equipment::ReGenerateModifier() {
        if (mp_StatGroup == nullptr) return;
        for (uint32_t i = 0; i < GetTraitCount(); ++i) {
            if (m_Traits[i].pModifier == nullptr) {
                m_Traits[i].pModifier = m_Traits[i].pRecreateModifierFunc(&mp_StatGroup->GetStat(m_Traits[i].Name), m_Traits[i].BaseValue);
            }
        }

        for (auto& baseStat : m_BaseStatistic) {
            baseStat.pModifier = baseStat.pRecreateModifierFunc(&mp_StatGroup->GetStat(baseStat.Name), baseStat.Value);
        }
    }

    Tier Equipment::GetRandomTierForTrait() const {
        return Random::WeightedDistribution(Equipment::TierWeightsByEquipmentTier[m_EquipmentTier], MAX_TIER);
    }

    Modifier * Equipment::CreateModifierAdd(Statistic *stat_name, float base) {
        return &stat_name->AddModifier<AddModifier>()->SetAddedValue(base);
    }

    Modifier * Equipment::CreateModifierMultiply(Statistic * stat_name, float base) {
        return &stat_name->AddModifier<MultiplyModifier>()->SetMultipliedValue(base);
    }

    Modifier * Equipment::CreateModifierAddFinal(Statistic *stat_name, float base) {
        return &stat_name->AddModifier<AddFinalModifier>()->SetAddedValue(base);
    }

    void Equipment::ReRollTraits(std::bitset<MAX_TIER> to_reroll) {
        if (CanReceiveGoldTrait() == false && to_reroll[m_GoldTrait]) m_GoldTrait = -1;

        for (uint32_t i = 0; i < GetTraitCount(); ++i) {
            if (to_reroll[i] == false) continue;

            auto modifier = GetTrait(i).pModifier;
            if (modifier != nullptr && mp_StatGroup != nullptr)
                mp_StatGroup->RemoveModifier(modifier);

            m_Traits[i] = CreateTraits(i, CanReceiveGoldTrait());
            if (m_Traits[i].TraitTier == MAX_TIER) m_GoldTrait = static_cast<int8_t>(i);
        }
    }

    void Equipment::EquipTo(StatisticGroup *pStatistiqueGrp) {
        if (mp_StatGroup != nullptr) {
            for (uint32_t i = 0; i < GetTraitCount(); ++i) {
                auto modifier = GetTrait(i).pModifier;
                if (modifier != nullptr)
                    mp_StatGroup->RemoveModifier(modifier);
            }
            for (auto& baseStat : m_BaseStatistic) {
                mp_StatGroup->RemoveModifier(baseStat.pModifier);
                baseStat.pModifier = nullptr;
            }
            mp_StatGroup->SetNotUpToDate();
        }

        mp_StatGroup = pStatistiqueGrp;
        if (mp_StatGroup != nullptr) {
            mp_StatGroup->SetNotUpToDate();
            ReGenerateModifier();
        }
    }

    Trait Helmet::CreateTraits(uint8_t index, bool can_be_t5) {
        constexpr  uint32_t ROLLABLE_COUNT = 6;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
                .Name = Statistic::STAT_NAME_HEALTH_POINT,
                .Weight = 1.0f,
                .RangeTier1 = { 10.0f, 20.0f },
                .RangeTier2 = { 40.0, 50.0f },
                .RangeTier3 = { 70.0, 80.0f },
                .RangeTier4 = { 90.0, 100.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ARMOR,
                .Weight = 1.1f,
                .RangeTier1 = { 1.0f, 2.0f },
                .RangeTier2 = { 3.0, 4.0f },
                .RangeTier3 = { 5.0, 6.0f },
                .RangeTier4 = { 7.0, 8.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_STRENGTH,
                .Weight = 1.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_SPEED,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_RESOURCE_MULT,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_PROVOCATION,
                .Weight = 0.5f,
                .RangeTier1 = { 0.02f, 0.07f },
                .RangeTier2 = { 0.07f, 0.12f },
                .RangeTier3 = { 0.12f, 0.17f },
                .RangeTier4 = { 0.17f, 0.22f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };
        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void Helmet::OnBaseStatistics() {

        m_BaseStatistic.resize(2);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_HEALTH_POINT,
            .Value = 5.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_ARMOR,
            .Value = 0.55f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait ChestPlate::CreateTraits(uint8_t index, bool can_be_t5)
    {
        constexpr  uint32_t ROLLABLE_COUNT = 6;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT]
        {
            {
                .Name = Statistic::STAT_NAME_HEALTH_POINT,
                .Weight = 1.0f,
                .RangeTier1 = { 20.0f, 30.0f },
                .RangeTier2 = { 45.0, 55.0f },
                .RangeTier3 = { 80.0, 90.0f },
                .RangeTier4 = { 100.0, 100.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ARMOR,
                .Weight = 1.1f,
                .RangeTier1 = { 2.0f, 4.0f },
                .RangeTier2 = { 5.0, 6.0f },
                .RangeTier3 = { 7.0, 8.0f },
                .RangeTier4 = { 9.0, 10.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_STRENGTH,
                .Weight = 1.5f,
                .RangeTier1 = { 0.03f, 0.04f },
                .RangeTier2 = { 0.05f, 0.06f },
                .RangeTier3 = { 0.07f, 0.08f },
                .RangeTier4 = { 0.09f, 0.10f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_SPEED,
                .Weight = 0.5f,
                .RangeTier1 = { 0.02f, 0.03f },
                .RangeTier2 = { 0.04f, 0.05f },
                .RangeTier3 = { 0.06f, 0.07f },
                .RangeTier4 = { 0.08f, 0.09f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_RESOURCE_MULT,
                .Weight = 0.5f,
                .RangeTier1 = { 0.02f, 0.03f },
                .RangeTier2 = { 0.04f, 0.05f },
                .RangeTier3 = { 0.06f, 0.07f },
                .RangeTier4 = { 0.08f, 0.09f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_PROVOCATION,
                .Weight = 0.5f,
                .RangeTier1 = { 0.05f, 0.010f },
                .RangeTier2 = { 0.010f, 0.15f },
                .RangeTier3 = { 0.15f, 0.20f },
                .RangeTier4 = { 0.20f, 0.25f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };

        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void ChestPlate::OnBaseStatistics()
    {
        m_BaseStatistic.resize(2);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_HEALTH_POINT,
            .Value = 5.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_ARMOR,
            .Value = 0.55f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait Gloves::CreateTraits(uint8_t index, bool can_be_t5)
    {
        constexpr  uint32_t ROLLABLE_COUNT = 6;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
            .Name = Statistic::STAT_NAME_HEALTH_POINT,
            .Weight = 0.8f,
            .RangeTier1 = { 5.0f,  15.0f },
            .RangeTier2 = { 25.0f, 35.0f },
            .RangeTier3 = { 55.0f, 65.0f },
            .RangeTier4 = { 75.0f, 85.0f },
            .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ARMOR,
                .Weight = 0.8f,
                .RangeTier1 = { 1.0f, 2.0f },
                .RangeTier2 = { 2.0f, 3.0f },
                .RangeTier3 = { 4.0f, 5.0f },
                .RangeTier4 = { 6.0f, 7.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_STRENGTH,
                .Weight = 1.5f,
                .RangeTier1 = { 0.02f, 0.04f },
                .RangeTier2 = { 0.05f, 0.07f },
                .RangeTier3 = { 0.08f, 0.10f },
                .RangeTier4 = { 0.11f, 0.13f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_SPEED,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_RESOURCE_MULT,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_PROVOCATION,
                .Weight = 0.5f,
                .RangeTier1 = { 0.02f, 0.06f },
                .RangeTier2 = { 0.06f, 0.11f },
                .RangeTier3 = { 0.11f, 0.16f },
                .RangeTier4 = { 0.16f, 0.21f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };

        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void Gloves::OnBaseStatistics() {
        m_BaseStatistic.resize(2);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_HEALTH_POINT,
            .Value = 2.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_ARMOR,
            .Value = 0.20f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait Legs::CreateTraits(uint8_t index, bool can_be_t5)
    {
        constexpr  uint32_t ROLLABLE_COUNT = 6;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
                .Name = Statistic::STAT_NAME_HEALTH_POINT,
                .Weight = 1.0f,
                .RangeTier1 = { 15.0f, 25.0f },
                .RangeTier2 = { 40.0f, 52.0f },
                .RangeTier3 = { 72.0f, 82.0f },
                .RangeTier4 = { 92.0f, 100.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ARMOR,
                .Weight = 1.1f,
                .RangeTier1 = { 1.0f, 3.0f },
                .RangeTier2 = { 4.0f, 5.0f },
                .RangeTier3 = { 6.0f, 7.0f },
                .RangeTier4 = { 8.0f, 9.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_STRENGTH,
                .Weight = 1.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_SPEED,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_RESOURCE_MULT,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_PROVOCATION,
                .Weight = 0.5f,
                .RangeTier1 = { 0.03f, 0.08f },
                .RangeTier2 = { 0.08f, 0.13f },
                .RangeTier3 = { 0.13f, 0.18f },
                .RangeTier4 = { 0.18f, 0.23f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };

        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void Legs::OnBaseStatistics() {
        m_BaseStatistic.resize(2);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_HEALTH_POINT,
            .Value = 4.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_ARMOR,
            .Value = 0.40f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait Boots::CreateTraits(uint8_t index, bool can_be_t5) {
        constexpr uint32_t ROLLABLE_COUNT = 6;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
                .Name = Statistic::STAT_NAME_HEALTH_POINT,
                .Weight = 0.8f,
                .RangeTier1 = { 5.0f,  15.0f },
                .RangeTier2 = { 25.0f, 35.0f },
                .RangeTier3 = { 55.0f, 65.0f },
                .RangeTier4 = { 75.0f, 85.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ARMOR,
                .Weight = 0.8f,
                .RangeTier1 = { 1.0f, 2.0f },
                .RangeTier2 = { 2.0f, 3.0f },
                .RangeTier3 = { 3.0f, 4.0f },
                .RangeTier4 = { 5.0f, 6.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_STRENGTH,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.02f, 0.03f },
                .RangeTier3 = { 0.03f, 0.05f },
                .RangeTier4 = { 0.05f, 0.07f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_SPEED,
                .Weight = 1.5f,
                .RangeTier1 = { 0.03f, 0.05f },
                .RangeTier2 = { 0.06f, 0.08f },
                .RangeTier3 = { 0.09f, 0.12f },
                .RangeTier4 = { 0.13f, 0.16f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_RESOURCE_MULT,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_PROVOCATION,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.05f },
                .RangeTier2 = { 0.05f, 0.10f },
                .RangeTier3 = { 0.10f, 0.15f },
                .RangeTier4 = { 0.15f, 0.20f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };

        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void Boots::OnBaseStatistics() {
        m_BaseStatistic.resize(2);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_HEALTH_POINT,
            .Value = 2.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_ARMOR,
            .Value = 0.15f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait ShortSword::CreateTraits(uint8_t index, bool can_be_t5) {

        constexpr  uint32_t ROLLABLE_COUNT = 5;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
                .Name = Statistic::STAT_NAME_DAMAGE,
                .Weight = 1.0f,
                .RangeTier1 = { 5.0f, 10.0f },
                .RangeTier2 = { 10.0, 20.0f },
                .RangeTier3 = { 20.0, 40.0f },
                .RangeTier4 = { 40.0, 80.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_CHANCE,
                .Weight = 1.1f,
                .RangeTier1 = { 2.0f, 5.0f },
                .RangeTier2 = { 5.0f, 12.0f },
                .RangeTier3 = { 12.0f, 20.0f },
                .RangeTier4 = { 20.0, 50.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_DMG,
                .Weight = 1.5f,
                .RangeTier1 = { 0.05f, 0.08f },
                .RangeTier2 = { 0.08f, 0.12f },
                .RangeTier3 = { 0.12f, 0.2f },
                .RangeTier4 = { 0.2f, 0.5f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ATTACK_SPEED,
                .Weight = 0.5f,
                .RangeTier1 = { 0.01f, 0.02f },
                .RangeTier2 = { 0.03f, 0.04f },
                .RangeTier3 = { 0.05f, 0.06f },
                .RangeTier4 = { 0.07f, 0.08f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_MELEE_RANGE,
                .Weight = 0.5f,
                .RangeTier1 = { 0.05f, 0.1f },
                .RangeTier2 = { 0.1f, 0.15f },
                .RangeTier3 = { 0.15f, 0.2f },
                .RangeTier4 = { 0.2f, 0.25f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };
        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void ShortSword::OnBaseStatistics() {
        m_BaseStatistic.resize(5);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_DAMAGE,
            .Value = 10.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_CHANCE,
            .Value = 30.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[2] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_DMG,
            .Value = 1.5f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[3] = {
            .Name = GPC::Statistic::STAT_NAME_ATTACK_SPEED,
            .Value = 1.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[4] = {
            .Name = GPC::Statistic::STAT_NAME_MELEE_RANGE,
            .Value = 1.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait LongSword::CreateTraits(uint8_t index, bool can_be_t5)
    {
        constexpr  uint32_t ROLLABLE_COUNT = 5;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
                .Name = Statistic::STAT_NAME_DAMAGE,
                .Weight = 1.0f,
                .RangeTier1 = { 8.0f,  16.0f },
                .RangeTier2 = { 16.0f, 32.0f },
                .RangeTier3 = { 32.0f, 64.0f },
                .RangeTier4 = { 64.0f, 128.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_CHANCE,
                .Weight = 0.8f,
                .RangeTier1 = { 1.0f,  3.0f },
                .RangeTier2 = { 3.0f,  8.0f },
                .RangeTier3 = { 8.0f,  15.0f },
                .RangeTier4 = { 15.0f, 35.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_DMG,
                .Weight = 1.5f,
                .RangeTier1 = { 0.08f, 0.12f },
                .RangeTier2 = { 0.12f, 0.20f },
                .RangeTier3 = { 0.20f, 0.35f },
                .RangeTier4 = { 0.35f, 0.65f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ATTACK_SPEED,
                .Weight = 0.5f,
                .RangeTier1 = { 0.005f, 0.010f },
                .RangeTier2 = { 0.010f, 0.020f },
                .RangeTier3 = { 0.020f, 0.040f },
                .RangeTier4 = { 0.040f, 0.060f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_MELEE_RANGE,
                .Weight = 1.1f,
                .RangeTier1 = { 0.10f, 0.15f },
                .RangeTier2 = { 0.15f, 0.22f },
                .RangeTier3 = { 0.22f, 0.30f },
                .RangeTier4 = { 0.30f, 0.40f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };
        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void LongSword::OnBaseStatistics() {
        m_BaseStatistic.resize(5);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_DAMAGE,
            .Value = 16.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_CHANCE,
            .Value = 18.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[2] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_DMG,
            .Value = 1.8f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[3] = {
            .Name = GPC::Statistic::STAT_NAME_ATTACK_SPEED,
            .Value = 0.7f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[4] = {
            .Name = GPC::Statistic::STAT_NAME_MELEE_RANGE,
            .Value = 1.5f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait Tomahawk::CreateTraits(uint8_t index, bool can_be_t5)
    {
        constexpr  uint32_t ROLLABLE_COUNT = 5;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
                .Name = Statistic::STAT_NAME_DAMAGE,
                .Weight = 1.0f,
                .RangeTier1 = { 10.0f,  20.0f },
                .RangeTier2 = { 20.0f,  40.0f },
                .RangeTier3 = { 40.0f,  80.0f },
                .RangeTier4 = { 80.0f, 150.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_CHANCE,
                .Weight = 0.7f,
                .RangeTier1 = { 1.0f,  2.0f },
                .RangeTier2 = { 2.0f,  5.0f },
                .RangeTier3 = { 5.0f, 10.0f },
                .RangeTier4 = { 10.0f, 20.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_DMG,
                .Weight = 1.5f,
                .RangeTier1 = { 0.10f, 0.15f },
                .RangeTier2 = { 0.15f, 0.25f },
                .RangeTier3 = { 0.25f, 0.40f },
                .RangeTier4 = { 0.40f, 0.70f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ATTACK_SPEED,
                .Weight = 0.5f,
                .RangeTier1 = { 0.003f, 0.007f },
                .RangeTier2 = { 0.007f, 0.012f },
                .RangeTier3 = { 0.012f, 0.020f },
                .RangeTier4 = { 0.020f, 0.035f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_MELEE_RANGE,
                .Weight = 0.5f,
                .RangeTier1 = { 0.08f, 0.12f },
                .RangeTier2 = { 0.12f, 0.18f },
                .RangeTier3 = { 0.18f, 0.25f },
                .RangeTier4 = { 0.25f, 0.35f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };
        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void Tomahawk::OnBaseStatistics() {
        m_BaseStatistic.resize(5);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_DAMAGE,
            .Value = 22.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_CHANCE,
            .Value = 12.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[2] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_DMG,
            .Value = 2.2f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[3] = {
            .Name = GPC::Statistic::STAT_NAME_ATTACK_SPEED,
            .Value = 0.5f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[4] = {
            .Name = GPC::Statistic::STAT_NAME_MELEE_RANGE,
            .Value = 1.2f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait Bow::CreateTraits(uint8_t index, bool can_be_t5)
    {
        constexpr  uint32_t ROLLABLE_COUNT = 5;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
                .Name = Statistic::STAT_NAME_DAMAGE,
                .Weight = 1.0f,
                .RangeTier1 = { 4.0f,  8.0f },
                .RangeTier2 = { 8.0f, 16.0f },
                .RangeTier3 = { 16.0f, 32.0f },
                .RangeTier4 = { 32.0f, 65.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_CHANCE,
                .Weight = 1.5f,
                .RangeTier1 = { 3.0f,  8.0f },
                .RangeTier2 = { 8.0f, 18.0f },
                .RangeTier3 = { 18.0f, 30.0f },
                .RangeTier4 = { 30.0f, 65.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_DMG,
                .Weight = 1.1f,
                .RangeTier1 = { 0.06f, 0.10f },
                .RangeTier2 = { 0.10f, 0.18f },
                .RangeTier3 = { 0.18f, 0.30f },
                .RangeTier4 = { 0.30f, 0.60f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ATTACK_SPEED,
                .Weight = 1.1f,
                .RangeTier1 = { 0.02f, 0.04f },
                .RangeTier2 = { 0.04f, 0.07f },
                .RangeTier3 = { 0.07f, 0.10f },
                .RangeTier4 = { 0.10f, 0.15f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_MELEE_RANGE,
                .Weight = 1.1f,
                .RangeTier1 = { 0.15f, 0.25f },
                .RangeTier2 = { 0.25f, 0.40f },
                .RangeTier3 = { 0.40f, 0.55f },
                .RangeTier4 = { 0.55f, 0.70f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };
        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void Bow::OnBaseStatistics() {
        m_BaseStatistic.resize(5);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_DAMAGE,
            .Value = 7.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_CHANCE,
            .Value = 45.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[2] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_DMG,
            .Value = 1.6f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[3] = {
            .Name = GPC::Statistic::STAT_NAME_ATTACK_SPEED,
            .Value = 1.5f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[4] = {
            .Name = GPC::Statistic::STAT_NAME_MELEE_RANGE,
            .Value = 3.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait Gauntlet::CreateTraits(uint8_t index, bool can_be_t5)
    {
        constexpr  uint32_t ROLLABLE_COUNT = 5;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
                .Name = Statistic::STAT_NAME_DAMAGE,
                .Weight = 1.0f,
                .RangeTier1 = { 4.0f,   8.0f },
                .RangeTier2 = { 8.0f,  16.0f },
                .RangeTier3 = { 16.0f, 32.0f },
                .RangeTier4 = { 32.0f, 64.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_CHANCE,
                .Weight = 1.0f,
                .RangeTier1 = { 2.0f,   5.0f },
                .RangeTier2 = { 5.0f,  10.0f },
                .RangeTier3 = { 10.0f, 20.0f },
                .RangeTier4 = { 20.0f, 40.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_DMG,
                .Weight = 0.8f,
                .RangeTier1 = { 0.05f, 0.10f },
                .RangeTier2 = { 0.10f, 0.18f },
                .RangeTier3 = { 0.18f, 0.30f },
                .RangeTier4 = { 0.30f, 0.55f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ATTACK_SPEED,
                .Weight = 1.4f,
                .RangeTier1 = { 0.020f, 0.040f },
                .RangeTier2 = { 0.040f, 0.070f },
                .RangeTier3 = { 0.070f, 0.110f },
                .RangeTier4 = { 0.110f, 0.160f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_MELEE_RANGE,
                .Weight = 0.6f,
                .RangeTier1 = { 0.05f, 0.10f },
                .RangeTier2 = { 0.10f, 0.16f },
                .RangeTier3 = { 0.16f, 0.24f },
                .RangeTier4 = { 0.24f, 0.34f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };
        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void Gauntlet::OnBaseStatistics() {
        m_BaseStatistic.resize(5);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_DAMAGE,
            .Value = 8.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_CHANCE,
            .Value = 25.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[2] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_DMG,
            .Value = 1.6f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[3] = {
            .Name = GPC::Statistic::STAT_NAME_ATTACK_SPEED,
            .Value = 1.4f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[4] = {
            .Name = GPC::Statistic::STAT_NAME_MELEE_RANGE,
            .Value = 0.8f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }

    Trait Axe::CreateTraits(uint8_t index, bool can_be_t5)
    {
        constexpr  uint32_t ROLLABLE_COUNT = 5;
        constexpr StatRollDescriptor STATS_ROLLABLE[ROLLABLE_COUNT] {
            {
                .Name = Statistic::STAT_NAME_DAMAGE,
                .Weight = 1.4f,
                .RangeTier1 = { 7.0f,  14.0f },
                .RangeTier2 = { 14.0f, 28.0f },
                .RangeTier3 = { 28.0f, 56.0f },
                .RangeTier4 = { 56.0f, 110.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_CHANCE,
                .Weight = 0.7f,
                .RangeTier1 = { 1.0f,   3.0f },
                .RangeTier2 = { 3.0f,   7.0f },
                .RangeTier3 = { 7.0f,  14.0f },
                .RangeTier4 = { 14.0f, 30.0f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_CRIT_DMG,
                .Weight = 1.6f,
                .RangeTier1 = { 0.10f, 0.18f },
                .RangeTier2 = { 0.18f, 0.30f },
                .RangeTier3 = { 0.30f, 0.50f },
                .RangeTier4 = { 0.50f, 0.85f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_ATTACK_SPEED,
                .Weight = 0.4f,
                .RangeTier1 = { 0.003f, 0.008f },
                .RangeTier2 = { 0.008f, 0.015f },
                .RangeTier3 = { 0.015f, 0.030f },
                .RangeTier4 = { 0.030f, 0.050f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            },
            {
                .Name = Statistic::STAT_NAME_MELEE_RANGE,
                .Weight = 1.0f,
                .RangeTier1 = { 0.08f, 0.14f },
                .RangeTier2 = { 0.14f, 0.20f },
                .RangeTier3 = { 0.20f, 0.28f },
                .RangeTier4 = { 0.28f, 0.38f },
                .pCreateModifierFunc = Equipment::CreateModifierAddFinal
            }
        };
        constexpr TraitRollDescriptor<ROLLABLE_COUNT> descriptor{STATS_ROLLABLE};

        Tier tier_chosen;
        do {
            tier_chosen = GetRandomTierForTrait();
        }
        while (can_be_t5 == false && tier_chosen == MAX_TIER);
        if (tier_chosen == MAX_TIER) {
            // TODO : Handle Tier 5 traits
        }

        uint32_t stat_chosen = Random::WeightedDistribution(descriptor.WEIGHTS, ROLLABLE_COUNT);
        float base = Random::Float(descriptor.RANGES[stat_chosen][tier_chosen].x, descriptor.RANGES[stat_chosen][tier_chosen].y);

        if (mp_StatGroup != nullptr) {
            return {
                .Name = descriptor.NAMES[stat_chosen],
                .TraitTier = tier_chosen,
                .BaseValue = base,
                .pModifier = descriptor.FUNCTIONS[stat_chosen](&mp_StatGroup->GetStat(descriptor.NAMES[stat_chosen]), base),
                .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
            };
        }
        return {
            .Name = descriptor.NAMES[stat_chosen],
            .TraitTier = tier_chosen,
            .BaseValue = base,
            .pModifier = nullptr,
            .pRecreateModifierFunc = descriptor.FUNCTIONS[stat_chosen]
        };
    }

    void Axe::OnBaseStatistics() {
        m_BaseStatistic.resize(5);

        m_BaseStatistic[0] = {
            .Name = GPC::Statistic::STAT_NAME_DAMAGE,
            .Value = 14.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[1] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_CHANCE,
            .Value = 20.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[2] = {
            .Name = GPC::Statistic::STAT_NAME_CRIT_DMG,
            .Value = 2.0f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[3] = {
            .Name = GPC::Statistic::STAT_NAME_ATTACK_SPEED,
            .Value = 0.8f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };

        m_BaseStatistic[4] = {
            .Name = GPC::Statistic::STAT_NAME_MELEE_RANGE,
            .Value = 1.2f,
            .pModifier = nullptr,
            .pRecreateModifierFunc = GPC::Equipment::CreateModifierAdd
        };
    }
} // GPC