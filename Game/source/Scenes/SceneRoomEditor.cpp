#include "Scenes/SceneRoomEditor.h"

#include "FreeCamera.h"
#include "Stream.h"
#include "DebugBehavior/AutoRotateDebug.h"
#include "RoomEditor/RoomEditorBehavior.h"
#include "RoomEditor/RoomPreviewBehavior.h"

namespace GPC {

    ErrorType SceneRoomEditor::OnAssetsLoad() {
        SceneDefault::OnAssetsLoad();

        GPC::MapGeneratorFactory::CreateFactory();

        return ErrorType::SUCCESS;
    }

    ErrorType SceneRoomEditor::OnCreate(SceneInformation &info) {
        GPC_SUCCESS(SceneDefault::OnCreate(info));

        info.pWindow->SetCursorMode(CursorState::DISABLED);

        auto preview = CreateEntityAs3D();
        auto pTransform = GetComponent<Transform3D>(preview);
        mp_Preview = AddBehavior<RoomPreviewBehavior>(preview);
        pTransform->LocalTransform.SetPosition(1000.0f, 0.0f, 0.0f);

        auto editor = CreateEntityAs3D();
        auto eTransform = GetComponent<Transform3D>(editor);
        mp_Editor = AddBehavior<RoomEditorBehavior>(editor);
        eTransform->LocalTransform.SetPosition(0.0f, 0.0f, 0.0f);

        mp_Preview->pEditor = mp_Editor;
        mp_Editor->pPreview = mp_Preview;

        auto lightTransform = GetComponent<Transform3D>(mp_DefaultLight->GetEntityID());
        lightTransform->LocalTransform.SetRotationYawPitchRoll(0, glm::radians(45.0f), 0.0f);

        return ErrorType::SUCCESS;
    }

    void SceneRoomEditor::OnDestroy() {

    }

} // GPC