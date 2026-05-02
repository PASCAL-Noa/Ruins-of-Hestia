
#include "Village/Buildings/Barrack/Barrack.h"

namespace GPC
{

    Barrack::Barrack() : Building(BuildingType::Barrack, "Icon_Comming_Soon",
        "CASERNE",
        "Entraine les guerrier",
        "Building_house",
        "Building_house", { 5.0f, 5.0f}, 5.0f, false)
    {
        ActivePath = "Building_house_active";
    }
} // GPC