//
// Created by wgsdp on 31/03/2026.
//

#pragma once

#include "SceneDefault.h"

namespace GPC
{
    class SceneSerializer : public SceneDefault
    {
        ErrorType OnCreate(SceneInformation &info) override;
        void OnDestroy() override;
    };
}