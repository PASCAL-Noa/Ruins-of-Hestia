
#pragma once
#include <string>

namespace GPC
{
    namespace Saves
    {

        using  SaveKey = const std::string;

        static constexpr SaveKey KEY_INVENTORY      = "Inventory";
        static constexpr SaveKey KEY_EXPEDITIONS    = "ExpeditionCount";
        static constexpr SaveKey KEY_BUILDINGS      = "Buildings";
        static constexpr SaveKey SAVE_FILE_PATH     = "VillageSave.gpc";

    }
}
