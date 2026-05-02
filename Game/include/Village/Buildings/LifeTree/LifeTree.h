#pragma once
#include "LifeTreeBehavior.h"
#include "../../Building.h"

namespace GPC
{
    class LifeTree : public Building
    {
    public:
        LifeTree();
        ~LifeTree() override = default;

        using BehaviorType = LifeTreeBehavior;
    };
}