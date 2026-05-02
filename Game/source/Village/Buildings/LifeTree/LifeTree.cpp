#include "Village/Buildings/LifeTree/LifeTree.h"

#include "Village/Resource.h"

namespace GPC
{
    LifeTree::LifeTree() : Building(BuildingType::LifeTree, "Building_LifeTree_0",
        "ARBRE DE VIE", "Le centre de votre village",
        "Building_LifeTree_0", "Building_LifeTree_0_highlight",
        {5, 5}, 5.0f, false)
    {
        SpriteDepthFactor   = 0.90f;
        SpriteLateralFactor = 0.0f;

        SetDependency<Food>(2);

        AddUpgradeResource<Wood<ResourceTier::T1>>(800);
        AddUpgradeResource<Stone<ResourceTier::T1>>(600);
        AddUpgradeResource<Metal<ResourceTier::T1>>(250);

    }

}
