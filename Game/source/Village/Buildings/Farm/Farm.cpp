#include "Village/Buildings/Farm/Farm.h"

namespace GPC
{
    Farm::Farm() :
        Building(BuildingType::Farm, "Icon_Ferme", "FERME", "Une petite ferme",
            "Building_farm", "Building_farm_highlight",
            {7, 9}, 1.45f, true)
    {

        ActivePath = "Building_farm_active";

        SpriteDepthFactor   = 0.75f;
        SpriteLateralFactor = 0.2f;

        SetDependency<Food>(10);
        
        AddUpgradeResource<Wood<ResourceTier::T1>>(600);
        AddUpgradeResource<Stone<ResourceTier::T1>>(400);

    }
}
