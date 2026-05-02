#pragma once
#include "HospitalBehavior.h"
#include "../../Building.h"

namespace GPC
{
    class Hospital : public Building
    {
    public:
        Hospital();
        ~Hospital() override = default;

        using BehaviorType = HospitalBehavior;

    };
}