#include "SceneManager.h"

#include "TransformComponents.h"

namespace GPC {

    ErrorType SceneManager::Create(RenderWindow *pWindow) {
        mp_RenderWindow = pWindow;
        return ErrorType::SUCCESS;
    }

    void SceneManager::Destroy() const {
        Transform3D::s_IsShuttingDown = true;
        for (auto pScene : mp_Scenes) {
            pScene->Destroy();
             //delete pScene;   scenes leak intentionally : "leak at exit" method
        }
    }

    void SceneManager::SetActiveScene(const std::string &name, SceneChangeCtx &ctx) {
        ShouldSwitchScene = true;
        NextScene = name;
        NextCtx = ctx;
    }

    Scene * SceneManager::GetCurrentScene() {
        return mp_Scenes[m_ActiveScene];
    }

    void SceneManager::UpdateCurrentScene() {
        if (ShouldSwitchScene) {
            mp_Scenes[m_ActiveScene]->OnEnd();

            m_ActiveScene = m_SceneIndices[NextScene];
            mp_Scenes[m_ActiveScene]->OnStart(NextCtx);
            ShouldSwitchScene = false;
        }
    }
} // GPC