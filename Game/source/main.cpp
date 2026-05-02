#include "Samples/SceneCollision.h"
#include "Scenes/SceneDefault.h"
#include "Scenes/SceneExemple.h"
#include "Scenes/SceneMeta.h"
#include "Scenes/SceneRoomEditor.h"
#include "Scenes/SceneFBX.h"
#include "Scenes/SceneMainMenu.h"
#include "Samples/SceneCollision.h"
#include "Samples/SceneCompute.h"
#include "Samples/SceneHDR.h"
#include "Samples/SceneShadows.h"
#include "Samples/SceneUiTest.h"
#include "Random.h"
#include "SceneManager.h"
#include "SerialFile.h"
#include "SerialTree.h"
#include "Expedition/Equipment.h"
#include "Scenes/SceneFBX.h"
#include "Scenes/SceneExpedition.h"
#include "Scenes/SceneMeta.h"
#include "Scenes/SceneSerializer.h"
#include "Scenes/SceneRoomEditor.h"

#include "Expedition/Stats.h"
#include "Samples/SceneAudio.h"
#include "Scenes/SceneVillageV1.h"
#include "Samples/SceneGraphics.h"
#include "Scenes/MadeWithScene.h"
#include "Scenes/SceneDialogue.h"
#include "Scenes/SceneExpedition.h"
#include "Scenes/SceneExpeditionPrep.h"

#define DURATION_SECONDS 20

int main(int argc, char *argv[]) {

#ifndef NDEBUG
    GPC::DebuggerDesc desc{};
    desc.output = GPC::DebuggerOutput::CONSOLE | GPC::DebuggerOutput::LOGS;
    GPC::Debugger::Init(&desc);
    GPC::Inputs::Create();
#endif

    srand(static_cast<uint32_t>(time(nullptr)));

    GPC::RenderWindow* window = new GPC::RenderWindow();
    GPC_SUCCESS(window->Create("Ruins of Hestia", 1080, 720));
    window->SetCursorMode(GPC::CursorState::NORMAL);
    window->Fullscreen(true);

    GPC::Assets::Create(window);

    GPC::SceneManager sceneManager{};
    sceneManager.Create(window);
    sceneManager.AddScene<GPC::SceneMadeWith>("Made_With");
    sceneManager.AddScene<GPC::SceneMainMenu>("Main_Menu");
    //sceneManager.AddScene<GPC::SceneRoomEditor>("Editor");
    sceneManager.AddScene<GPC::SceneVillageV1>("Village");
    sceneManager.AddScene<GPC::SceneExpeditionPrep>("ExpeditionPrep");
    sceneManager.AddScene<GPC::SceneExpedition>("Expedition");
    // sceneManager.AddScene<GPC::SceneUiTest>("UiTest");
    // sceneManager.AddScene<GPC::SceneAudio>("Audio");

    GPC::SceneExpeditionStartCtx* pCtx = new GPC::SceneExpeditionStartCtx();
    pCtx->MissionDifficulty = 1;

    GPC::SceneChangeCtx ctx{};
    sceneManager.SetActiveScene("Made_With", ctx);

    while (
        !window->ShouldClose() &&
        !GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::ESCAPE)
        )
    {
        sceneManager.GetCurrentScene()->Update();
    }

    // Destruction
    sceneManager.Destroy();

    GPC::RenderDevice::WaitIdle();
    GPC::Assets::DestroyAllAssets();
    GPC::MapGeneratorFactory::DestroyAll();

    window->Destroy();
    delete window;

#ifndef NDEBUG
    GPC::Debugger::Destroy();
#endif // NDEBUG

    return 0;
}
