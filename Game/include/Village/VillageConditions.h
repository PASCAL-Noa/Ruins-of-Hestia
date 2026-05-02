#pragma once
#include "Condition.h"
#include "Map/Editor.h"

namespace GPC
{
    class VillageController;

    class ConditionInputBack : public Condition<VillageController>
    {
    public:
        bool OnTest(VillageController* pOwner) override
        {
            return !pOwner->IsEditing() && !pOwner->IsBuilding() && !pOwner->IsInspecting();
        }
    };

    class ConditionInputSetting : public Condition<VillageController>
    {
    public:
        bool OnTest(VillageController* pOwner) override
        {
            return false;
        }
    };

    class ConditionInputEdit : public Condition<VillageController>
    {
    public:
        bool OnTest(VillageController* pOwner) override
        {
            return pOwner->IsEditing();
        }
    };

    class ConditionInputInspect : public Condition<VillageController>
    {
    public:
        bool OnTest(VillageController* pOwner) override
        {
            return pOwner->IsInspecting();
        }
    };
}
