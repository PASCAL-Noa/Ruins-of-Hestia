#pragma once
#include "../Scenes/SceneDefault.h"

namespace GPC {

    class SceneTransformTest : public SceneDefault {
        Transform3D* mp_Obj1 = nullptr;
        Transform3D* mp_Obj2 = nullptr;
        Transform3D* mp_Obj3 = nullptr;
    public:
        ErrorType OnCreate(SceneInformation &info) override;
        void OnDestroy() override;
    };

} // GPC