
#pragma once
#include "BarrackBehavior.h"
#include "Village/Building.h"

namespace GPC
{
    class Barrack : public Building
    {

    public:
        Barrack();
        ~Barrack() override = default;

        using BehaviorType = BarrackBehavior;

    private:
        friend class BarrackBehavior;
    };

} // GPC