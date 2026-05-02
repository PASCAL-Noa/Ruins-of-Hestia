#include "../../include/Samples/SceneTransformTest.h"

#include "FreeCamera.h"
#include "LightingSystem.h"
#include "Render3DSystem.h"
#include "DebugBehavior/MoveDebug.h"
#include "DebugBehavior/RotationDebug.h"

namespace GPC {
    ErrorType SceneTransformTest::OnCreate(SceneInformation &info) {
        GPC_SUCCESS(SceneDefault::OnCreate(info));

        /*
         * Cette Scene permet de mettre en oeuvre les relations relative des différents transforms
         * Ici vous trouverez :
         * - un plan au sol qui est fixe, il sert de référenciel
         * - deux sphere, la sphere en carbon est l'object1, la sphere avec la texture de Debug est l'object2
         * - une camera
         *
         * Les Transform3D des objects sont hiéarchisé comme suis :
         *  Object 1
         *  |=> Object 2
         *  |=> Camera
         *  Plan
         *
         *  KeyBinds :
         *  - ZQSD => bouger l'Object 1
         *  - AE => tourner l'Object 1
         *  - FG => tourner l'Object 2
         */

        RemoveBehavior<FreeCamera>(mp_MainCamera->GetEntityID());

        auto obj1 = CreateEntityAs3D();
        auto obj2 = CreateEntityAs3D();
        auto obj3 = CreateEntityAs3D();

        // Transform3D
        mp_Obj1 = GetComponent<Transform3D>(obj1);
        mp_Obj2 = GetComponent<Transform3D>(obj2);
        mp_Obj3 = GetComponent<Transform3D>(obj3);
        mp_Obj1->LocalTransform.SetPosition(0, 0, 0);
        mp_Obj2->SetParent(mp_Obj1);
        mp_Obj2->LocalTransform.SetPosition(1, 1, 0);
        mp_Obj3->LocalTransform.SetPosition(0, -1, 0);
        mp_Obj3->LocalTransform.SetScale(5, 1, 5);
        mp_MainCameraTransform->SetParent(mp_Obj1);
        mp_MainCameraTransform->LocalTransform.SetRotationYawPitchRoll({glm::radians(45.0f), glm::radians(-45.0f), 0});
        mp_MainCameraTransform->LocalTransform.AddPosition(mp_MainCameraTransform->LocalTransform.GetForward() * 4);

        // Render3D
        auto oRender1 = AddComponent<Render3DComponent>(obj1);
        auto oRender2 = AddComponent<Render3DComponent>(obj2);
        auto oRender3 = AddComponent<Render3DComponent>(obj3);
        oRender1->pGeometry = GPC_MESH("Sphere");
        oRender1->RenderMaterial.Diffuse = GPC_TEXTURE_ID("1024x1024", "Carbon");
        oRender2->pGeometry = GPC_MESH("Sphere");
        oRender2->RenderMaterial.Diffuse = GPC_TEXTURE_ID("1024x1024", "Debug");
        oRender3->pGeometry = GPC_MESH("Plane");
        oRender3->RenderMaterial.Diffuse = GPC_TEXTURE_ID("1024x1024", "Parquet");

        // Lights
        auto oLight1 = AddComponent<LightingComponent>(obj1);
        auto oLight2 = AddComponent<LightingComponent>(obj2);
        oLight1->pLightData = GPC_POINT_LIGHT("PointWhite");
        oLight2->pLightData = GPC_POINT_LIGHT("PointBlue");

        auto movedbg1 = AddBehavior<MoveDebug>(obj1);
        auto movedbg2 = AddBehavior<MoveDebug>(obj2);
        auto rotatedbg1 = AddBehavior<RotationDebug>(obj1);
        auto rotatedbg2 = AddBehavior<RotationDebug>(obj2);

        movedbg1->KeyLeft = Inputs::KeyCode::A;
        movedbg1->KeyRight = Inputs::KeyCode::D;
        movedbg1->KeyUp = Inputs::KeyCode::W;
        movedbg1->KeyDown = Inputs::KeyCode::S;
        rotatedbg1->RotateLeftKey = Inputs::KeyCode::Q;
        rotatedbg1->RotateRightKey = Inputs::KeyCode::E;

        movedbg2->KeyLeft = Inputs::KeyCode::LEFT;
        movedbg2->KeyRight = Inputs::KeyCode::RIGHT;
        movedbg2->KeyUp = Inputs::KeyCode::UP;
        movedbg2->KeyDown = Inputs::KeyCode::DOWN;
        rotatedbg2->RotateLeftKey = Inputs::KeyCode::O;
        rotatedbg2->RotateRightKey = Inputs::KeyCode::P;

        return ErrorType::SUCCESS;
    }

    void SceneTransformTest::OnDestroy() {

    };
} // GPC