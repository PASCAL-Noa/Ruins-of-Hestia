#include "Village/Buildings/Hospital/Hospital.h"

namespace GPC
{
    Hospital::Hospital() : Building(BuildingType::Hospital, "Icon_Comming_Soon", "HOPITAL", "L'hopital du village",
        "Building_house", "Building_house_highlight",
        {2, 2}, 2.0f, false)
    {
        ActivePath = "Building_house_active";
    }

}
