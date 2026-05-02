#pragma once

#include <cstdint>

#ifndef NDEBUG
#define ASSET_VERBOSE(verbose, level) if (verbose >= level)
#else
#define ASSET_VERBOSE(verbose, level) if (false)
#endif


namespace GPC {

    enum class VerboseLevel : uint32_t
    {
        NONE            = 0x00000000,
        CRITIQUE        = 0x0000000F,
        IMPORTANT       = 0x000000FF,
        DETAIL          = 0x00000FFF,
        DEBUG           = 0x0000FFFF,
        ALL             = 0xFFFFFFFF
    };

}

