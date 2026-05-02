#include "Village/Buildings/Forge/Forge.h"

#include "Village/Resource.h"

namespace GPC
{

    Forge::Forge() : Building(BuildingType::Forge, "Icon_Forge", "FORGE", "La forge du village",
        "Building_forge", "Building_forge_highlight",
        {5, 5}, 2.0f, true)
    {
        ActivePath = "Building_forge_active";

        SpriteDepthFactor   = 0.78f;
        SpriteLateralFactor = 0.05f;

        AddUpgradeResource<Wood<ResourceTier::T1>>(200);
        AddUpgradeResource<Stone<ResourceTier::T1>>(400);
        AddUpgradeResource<Metal<ResourceTier::T1>>(800);

    }

}
