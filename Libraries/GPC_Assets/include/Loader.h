//
// Created by hugoc on 02/03/2026.
//

#pragma once

#include <string>
#include "Assets.h"

namespace GPC
{
constexpr int32_t RGBA = 4;

    class Loader
    {

        public:
        Loader() = default;

        static bool LoadTexture(const std::string& filePath, TextureAsset* asset);
        static void LoadAtlas(const std::string& filePath, Assets* manager);

    };
}





