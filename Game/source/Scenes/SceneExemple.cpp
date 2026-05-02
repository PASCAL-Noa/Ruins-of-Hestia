#include "Scenes/SceneExemple.h"

#include "FreeCamera.h"
#include "LightingSystem.h"
#include "Render3DSystem.h"

namespace GPC
{
    ErrorType SceneExemple::OnCreate(SceneInformation &info)
    {
        SceneDefault::OnCreate(info);
        auto cController    = AddBehavior<FreeCamera>(mp_MainCamera->GetEntityID());    // On donne un script qui fait bouger la camera

        cController->MovementSpeed = 10; // Si on change un paramètre exposé d'une behaviour il s'update en live

        auto player  = CreateEntityAs3D();

        auto pRenderer      = AddComponent<Render3DComponent>(player);
        pRenderer->pGeometry = GPC_MESH("Cube"); // Trois type possible "Cube/Sphere/Plane"
        // "1024x1024"  -> Refer to GPC_ASSETS->AddTextureArray("1024x1024", ...)
        // "Debug"      -> Refer to GPC_ASSETS->AddTexture("Debug", ...)
        pRenderer->RenderMaterial.Diffuse = GPC_TEXTURE_ID("1024x1024", "Debug");

        auto light0 = CreateEntityAs3D();
        auto lLight0 = AddComponent<LightingComponent>(light0);
        auto lTransform0 = GetComponent<Transform3D>(light0);
        lLight0->pLightData = GPC_POINT_LIGHT("PointWhite");
        lTransform0->LocalTransform.AddPosition(2, 3, 2);
        return ErrorType::SUCCESS;
    }

    void SceneExemple::OnDestroy()
    {

    }
}
