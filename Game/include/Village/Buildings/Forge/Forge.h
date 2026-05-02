#pragma once
#include "ForgeBehavior.h"
#include "../../Building.h"

namespace GPC
{
    class Forge : public Building
    {
    public:
        Forge();
        ~Forge() override = default;

        using BehaviorType = ForgeBehavior;

    };
}