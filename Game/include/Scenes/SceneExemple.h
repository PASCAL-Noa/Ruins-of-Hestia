#pragma once

#include "SceneDefault.h"

namespace GPC
{
    class SceneExemple : public SceneDefault
    {

        ErrorType OnCreate(SceneInformation &info) override;
        void OnDestroy() override;

    };
}
