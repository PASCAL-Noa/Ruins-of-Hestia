#pragma once
#include "FarmBehavior.h"
#include "../../Building.h"

namespace GPC
{
    class Farm : public Building
    {
    public:
        Farm();
        ~Farm() override = default;

        using BehaviorType = FarmBehavior;
    private:
        friend class FarmBehavior;

    };
}
