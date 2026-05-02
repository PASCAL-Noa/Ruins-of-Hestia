#include "Village/Buildings/House/House.h"

#include "Village/Resource.h"

namespace GPC
{
    House::House() : Building(BuildingType::House, "Icon_Habitat", "MAISON",
        "Repose toi là !", "Building_house", "Building_house_highlight",
        {5, 5}, 2.0f, true)
    {

        ActivePath = "Building_house_active";

        SpriteDepthFactor   = 0.70f;
        SpriteLateralFactor = 0.0f;

        SetDependency<Food>(-2);

        AddUpgradeResource<Wood<ResourceTier::T1>>(400);
        AddUpgradeResource<Stone<ResourceTier::T1>>(200);

    }

}
