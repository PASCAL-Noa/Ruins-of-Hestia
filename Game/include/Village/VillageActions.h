#pragma once
#include "Action.h"

namespace GPC
{
    class VillageController;

    class ActionToggleScroll : public Action<VillageController>
    {
    public:
        bool Enable = true;
        void Start(VillageController* pOwner) override;
        void Update(VillageController* pOwner) override {}
        void End(VillageController* pOwner) override {}
    };

    class ActionToggleMainUI : public Action<VillageController>
    {
    public:
        bool Enable = true;
        void Start(VillageController* pOwner) override;
        void Update(VillageController* pOwner) override {}
        void End(VillageController* pOwner) override {}
    };

    class ActionToggleEditUI : public Action<VillageController>
    {
    public:
        bool Enable = true;
        void Start(VillageController* pOwner) override;
        void Update(VillageController* pOwner) override {}
        void End(VillageController* pOwner) override {}
    };

    class ActionToggleInspectUI : public Action<VillageController>
    {
    public:
        bool Enable = true;
        void Start(VillageController* pOwner) override;
        void Update(VillageController* pOwner) override {}
        void End(VillageController* pOwner) override {}
    };

    class ActionToggleBuildingEdit : public Action<VillageController>
    {
    public:
        bool Enable = true;
        void Start(VillageController* pOwner) override;
        void Update(VillageController* pOwner) override {}
        void End(VillageController* pOwner) override {}
    };

    class ActionToggleBuildingInteract : public Action<VillageController>
    {
    public:
        bool Enable = true;
        void Start(VillageController* pOwner) override;
        void Update(VillageController* pOwner) override {}
        void End(VillageController* pOwner) override {}
    };
}