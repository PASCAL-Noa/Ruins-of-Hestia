#pragma once

#include "GPC_Framework.h"
#include <map>
#include <string>
#include <vector>

namespace GPC {

    class Statistic;
    struct Modifier;
    struct StatDescription;
    class StatisticGroup;
    struct ModifierContext;

    enum class ModifierPriority {
        FIRST       = 0,
        SECOND      = 1,
        FINAL       = 2,
        COUNT
    };

    using ModifierGroup = std::array<std::vector<Modifier*>, static_cast<uint32_t>(ModifierPriority::COUNT)>;

    struct ModifierContext {
        StatisticGroup* pGroup;
        const Statistic* pStatistic;
    };

    class Statistic {
        float m_BaseValue                           = .0f;
        float m_FinalValue                          = .0f;
        ModifierGroup m_ModifierGroup               = {};
        StatisticGroup* mp_Group                    = nullptr;
        bool m_FinalValueUpToDate                   = false;

        void UpdateFinalValue();

    public:

        // Entity Stats
        static constexpr char STAT_NAME_HEALTH_POINT[]              = "pv";
        static constexpr char STAT_NAME_ARMOR[]                     = "armure";
        static constexpr char STAT_NAME_STRENGTH[]                  = "force";
        static constexpr char STAT_NAME_SPEED[]                     = "vitesse";
        static constexpr char STAT_NAME_RESOURCE_MULT[]             = "resource multiplier";
        static constexpr char STAT_NAME_PROVOCATION[]               = "provocation";

        // Weapon Stats
        static constexpr char STAT_NAME_DAMAGE[]                    = "dégats";
        static constexpr char STAT_NAME_CRIT_CHANCE[]               = "crit %";
        static constexpr char STAT_NAME_CRIT_DMG[]                  = "dégats crit ";
        static constexpr char STAT_NAME_ATTACK_SPEED[]              = "vitesse attaque";

        // Melee Weapon Stats
        static constexpr char STAT_NAME_MELEE_RANGE[]               = "melee range";

        // Distance Weapon Stats
        static constexpr char STAT_NAME_DISTANCE_PROJECTILE_SPEED[] = "dist projectile_speed";
        static constexpr char STAT_NAME_DISTANCE_PENETRATION[]      = "dist penetration";
        static constexpr char STAT_NAME_DISTANCE_PROJECTILE_SIZE[]  = "dist projectile size";

        const std::string Name;
        Statistic(std::string_view name, float base, StatisticGroup& grp);

        void ResetBaseValue(float value) { m_BaseValue = value; };

        [[nodiscard]] float GetBaseValue() const { return m_BaseValue; };
        [[nodiscard]] float GetFinalValue();

        void ResetFinalValue();

        template<typename Modifier_t> requires(std::is_base_of_v<Modifier, Modifier_t>)
        Modifier_t* AddModifier();

        void RemoveModifier(Modifier* ptr);

        void ClearModifiers();
    };


    struct Modifier {
        const ModifierPriority Priority;
        const Statistic* pStatistic;
        virtual ~Modifier() = default;
        explicit Modifier(ModifierContext& ctx, ModifierPriority priority) : Priority(priority), pStatistic(ctx.pStatistic) { }

        virtual void OnCreate(ModifierContext& ctx) { };
        virtual float OnModifierApplication(float previous, ModifierContext& ctx) const = 0;
        virtual Modifier* CreateCopyTo(Statistic* pStat) = 0;
        virtual std::string GetAsString() = 0;
    };

    class StatisticGroup {
        std::map<std::string, Statistic> m_Statistics;
    public:
        Statistic& AddStat(std::string name, float base);
        Statistic& GetStat(std::string name);
        float GetStatValue(std::string name);

        void RemoveModifier(Modifier* ptr);

        void SetNotUpToDate();

        [[nodiscard]] const std::map<std::string, Statistic>& GetStatistics() const { return m_Statistics; }
    };

    template<typename Modifier_t> requires(std::is_base_of_v<Modifier, Modifier_t>)
    Modifier_t * Statistic::AddModifier() {
        ModifierContext ctx{};
        ctx.pGroup = mp_Group;
        ctx.pStatistic = this;
        auto pModif = new Modifier_t(ctx);
        m_ModifierGroup[static_cast<uint32_t>(pModif->Priority)].push_back(pModif);
        pModif->OnCreate(ctx);
        ResetFinalValue();
        return pModif;
    }

    class AddModifier : public Modifier {
        float m_AddValue = 0.0f;
    public:
        ~AddModifier() override = default;
        explicit AddModifier(ModifierContext& ctx) : Modifier(ctx, ModifierPriority::FIRST) { }
        float OnModifierApplication(float previous, ModifierContext& ctx) const override;

        AddModifier& SetAddedValue(float value) { m_AddValue = value; return *this; }
        Modifier* CreateCopyTo(Statistic* pStat) override;
        std::string GetAsString() override { return "+ " + std::to_string(m_AddValue) + " (" + pStatistic->Name + ")"; }
    };

    class MultiplyModifier : public Modifier {
        float m_Multiplier = 1.0f;
    public:
        ~MultiplyModifier() override = default;
        explicit MultiplyModifier(ModifierContext& ctx) : Modifier(ctx, ModifierPriority::SECOND) { }
        float OnModifierApplication(float previous, ModifierContext& ctx) const override;

        MultiplyModifier& SetMultipliedValue(float value) { m_Multiplier = value; return *this; }
        Modifier* CreateCopyTo(Statistic* pStat) override;
        std::string GetAsString() override { return "x " + std::to_string(m_Multiplier) + " (" + pStatistic->Name + ")"; }
    };

    class AddFinalModifier : public Modifier {
        float m_AddValue = 0.0f;
    public:
        ~AddFinalModifier() override = default;
        explicit AddFinalModifier(ModifierContext& ctx) : Modifier(ctx, ModifierPriority::FINAL) { }
        float OnModifierApplication(float previous, ModifierContext& ctx) const override;

        AddFinalModifier& SetAddedValue(float value) { m_AddValue = value; return *this; }
        Modifier* CreateCopyTo(Statistic* pStat) override;
        std::string GetAsString() override { return "+ " + std::to_string(m_AddValue) + " (" + pStatistic->Name + ")"; }
    };

} // GPC
