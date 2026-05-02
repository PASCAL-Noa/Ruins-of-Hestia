#pragma once

#include "Map Generation/Room.h"
#include "SceneDefault.h"

#include "Render3DSystem.h"
#include "RoomEditor/RoomMeta.h"
#include "RoomEditor/RoomPreviewBehavior.h"
#include "RoomEditor/SpawnerDebug.h"

namespace GPC {

    class SceneRoomEditor : public SceneDefault {

        RoomPreviewBehavior* mp_Preview = nullptr;

        RoomEditorBehavior* mp_Editor      = nullptr;

    public:
        ErrorType OnAssetsLoad() override;
        ErrorType OnCreate(SceneInformation &info) override;
        void OnDestroy() override;

    };

} // GPC