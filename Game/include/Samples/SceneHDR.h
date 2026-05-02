#pragma once
#include "Scenes/SceneDefault.h"

namespace GPC
{
    class SceneHDR : public SceneDefault
    {
    public:
        ErrorType OnAssetsLoad() override;
        void OnDestroy() override;
        ErrorType OnCreate(SceneInformation &info) override;
    };
} // GPC