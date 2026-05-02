//
// Created by killi on 25/03/2026.
//

#ifndef RUINS_OF_HESTIA_SCENEMETA_H
#define RUINS_OF_HESTIA_SCENEMETA_H

#include "Scene.h"
#include "Assets.h"

namespace GPC
{
    class SceneMeta : public Scene
    {
    public:
        ErrorType OnAssetsLoad() override;
        void OnAssetsDestroy() override;

        ErrorType OnCreate(SceneInformation& info) override;
        void OnDestroy() override;
    };
} // GPC

#endif //RUINS_OF_HESTIA_SCENEMETA_H