#pragma once
#include "GPC_Framework.h"

enum PlantState
{
    Idle = 0,
    Shooting,
    Reloading,
    Count
};

struct PlantContext
{
    int Ammo = 3;
    int MaxAmmo = 3;
    bool ZombieOnLane = false;
    int ReloadTicks = 0;

    int IdleStart = 0;
    int IdleEnd = 0;
    int ShootStart = 0;
    int ShootEnd = 0;
    int ReloadStart = 0;
    int ReloadEnd = 0;
};

class IdleAction final : public GPC::Action<PlantContext>
{
public:
    void Start(PlantContext* p) override { ++p->IdleStart; }
    void Update(PlantContext*) override {}
    void End(PlantContext* p) override { ++p->IdleEnd; }
};

class ShootingAction final : public GPC::Action<PlantContext>
{
public:
    void Start(PlantContext* p) override { ++p->ShootStart; }
    void Update(PlantContext* p) override
    {
        if (p->Ammo > 0) { --p->Ammo; }
    }
    void End(PlantContext* p) override { ++p->ShootEnd; }
};

class ReloadingAction final : public GPC::Action<PlantContext>
{
public:
    void Start(PlantContext* p) override
    {
        ++p->ReloadStart;
        p->ReloadTicks = 0;
    }

    void Update(PlantContext* p) override
    {
        ++p->ReloadTicks;
        if (p->ReloadTicks >= 2) { p->Ammo = p->MaxAmmo; }
    }

    void End(PlantContext* p) override { ++p->ReloadEnd; }
};

class ZombieOnLaneCondition final : public GPC::Condition<PlantContext>
{
public:
    bool OnTest(PlantContext* p) override { return p->ZombieOnLane; }
};

class NoAmmoCondition final : public GPC::Condition<PlantContext>
{
public:
    bool OnTest(PlantContext* p) override { return p->Ammo == 0; }
};

class FullAmmoCondition final : public GPC::Condition<PlantContext>
{
public:
    bool OnTest(PlantContext* p) override { return p->Ammo == p->MaxAmmo; }
};


namespace STATE_MACHINE_PLANT_TEST
{
    static void StateMachine_PlantTest()
    {
        PlantContext plantContext;
        GPC::StateMachine<PlantContext> stateMachine(&plantContext, PlantState::Count);

        auto* idle = stateMachine.CreateBehaviour(PlantState::Idle);
        [[maybe_unused]] auto* idleAction = idle->AddAction<IdleAction>();

        idle->CreateTransition(PlantState::Shooting)
            ->AddCondition<ZombieOnLaneCondition>();

        auto* shooting = stateMachine.CreateBehaviour(PlantState::Shooting);
        shooting->AddAction<ShootingAction>();

        {
            auto* zombieOnLaneCondition = shooting->CreateTransition(PlantState::Idle)
                ->AddCondition<ZombieOnLaneCondition>();
            zombieOnLaneCondition->Expected = false;
        }

        shooting->CreateTransition(PlantState::Reloading)
            ->AddCondition<NoAmmoCondition>();

        auto* reloading = stateMachine.CreateBehaviour(PlantState::Reloading);
        reloading->AddAction<ReloadingAction>();

        {
            auto* transition = reloading->CreateTransition(PlantState::Idle);
            transition->AddCondition<FullAmmoCondition>();
            auto* zombieOnLaneCondition = transition->AddCondition<ZombieOnLaneCondition>();
            zombieOnLaneCondition->Expected = false;
        }

        stateMachine.SetState(PlantState::Idle);
        assert(stateMachine.GetCurrentState() == PlantState::Idle);
        assert(plantContext.IdleStart == 1);

        plantContext.ZombieOnLane = true;
        stateMachine.Update();
        assert(stateMachine.GetCurrentState() == PlantState::Shooting);
        assert(plantContext.ShootStart == 1);

        stateMachine.Update();
        stateMachine.SetState(STATE_MACHINE_PAUSE);
        stateMachine.Update();
        stateMachine.Update();
        stateMachine.SetState(1);
        stateMachine.Update();
        stateMachine.Update();
        assert(plantContext.Ammo == 0);
        assert(stateMachine.GetCurrentState() == PlantState::Shooting);

        stateMachine.Update();
        assert(stateMachine.GetCurrentState() == PlantState::Reloading);
        assert(plantContext.ReloadStart == 1);

        stateMachine.Update();
        stateMachine.Update();
        assert(plantContext.Ammo == plantContext.MaxAmmo);

        plantContext.ZombieOnLane = false;
        stateMachine.Update();
        assert(stateMachine.GetCurrentState() == PlantState::Idle);

        GPC_INFO << "Plant StateMachine test passed.\n";
    };
}