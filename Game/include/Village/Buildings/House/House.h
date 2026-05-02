#pragma once
#include "HouseBehaviour.h"
#include "../../Building.h"

namespace GPC
{
    class House : public Building
    {
    public:
        House();
        ~House() override = default;

        using BehaviorType = HouseBehaviour;

    };
}