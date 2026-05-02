#include "Expedition/Stats.h"

namespace GPC {

    void Statistic::UpdateFinalValue() {
        if (m_FinalValueUpToDate) return;

        m_FinalValue = GetBaseValue();
        ModifierContext ctx{};
        ctx.pGroup = mp_Group;
        ctx.pStatistic = this;

        for (uint32_t i = 0; i < static_cast<uint32_t>(ModifierPriority::COUNT); ++i) {
            for (auto pModifier : m_ModifierGroup[i]) {
                m_FinalValue = pModifier->OnModifierApplication(m_FinalValue, ctx);
            }
        }

    }

    Statistic::Statistic(std::string_view name, float base, StatisticGroup &grp) :
        m_BaseValue(base),
        mp_Group(&grp),
        Name(name)
    {}

    float Statistic::GetFinalValue() {
        UpdateFinalValue();
        return m_FinalValue;
    }

    void Statistic::ResetFinalValue() {
        m_FinalValueUpToDate = false;
    }

    void Statistic::RemoveModifier(Modifier *ptr) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(ModifierPriority::COUNT); ++i) {
            for (auto it = m_ModifierGroup[i].begin(); it !=  m_ModifierGroup[i].end(); ++it) {
                if (*it == ptr) {
                    delete ptr;
                    m_ModifierGroup[i].erase(it);
                    return;
                }
            }
        }
    }

    void Statistic::ClearModifiers() {
        for (uint32_t i = 0; i < static_cast<uint32_t>(ModifierPriority::COUNT); ++i) {
            for (auto pModifier : m_ModifierGroup[i]) {
                delete pModifier;
            }
            m_ModifierGroup[i].clear();
        }
        ResetFinalValue();
    }

    Statistic& StatisticGroup::AddStat(std::string name, float base) {
        if (m_Statistics.contains(name)) {
            auto& stat = m_Statistics.at(name);
            stat.ResetBaseValue(base);
            return stat;
        }
        m_Statistics.insert({ name, Statistic{name, base, *this} });
        return m_Statistics.at(name);
    }

    Statistic & StatisticGroup::GetStat(std::string name) {
        return m_Statistics.at(name);
    }

    float StatisticGroup::GetStatValue(std::string name) {
        if (m_Statistics.contains(name) == false) return 0.0f;
        return m_Statistics.at(name).GetFinalValue();
    }

    void StatisticGroup::RemoveModifier(Modifier *ptr) {
        for (auto& stat : m_Statistics) {
            stat.second.RemoveModifier(ptr);
        }
    }

    void StatisticGroup::SetNotUpToDate() {
        for (auto& stat : m_Statistics) {
            stat.second.ResetFinalValue();
        }
    }

    float AddModifier::OnModifierApplication(float previous, ModifierContext &ctx) const {
        return previous + m_AddValue;
    }

    Modifier * AddModifier::CreateCopyTo(Statistic *pStat) {
        return &pStat->AddModifier<AddModifier>()->SetAddedValue(m_AddValue);
    }

    float MultiplyModifier::OnModifierApplication(float previous, ModifierContext &ctx) const {
        return previous * m_Multiplier;
    }

    Modifier * MultiplyModifier::CreateCopyTo(Statistic *pStat) {
        return &pStat->AddModifier<MultiplyModifier>()->SetMultipliedValue(m_Multiplier);
    }

    float AddFinalModifier::OnModifierApplication(float previous, ModifierContext &ctx) const {
        return previous + m_AddValue;
    }

    Modifier * AddFinalModifier::CreateCopyTo(Statistic *pStat) {
        return &pStat->AddModifier<AddFinalModifier>()->SetAddedValue(m_AddValue);
    }
} // GPC