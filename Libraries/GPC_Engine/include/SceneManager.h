#pragma once
#include <vector>

#include "RenderWindow.h"
#include "Object.h"
#include "Scene.h"

namespace GPC {

    using SceneID = uint8_t;

    GPC_INHERIT_OBJECT(SceneManager) {

        struct SceneStatus {
            Scene* pScene;
            bool Created;
        };

        RenderWindow* mp_RenderWindow;
        uint32_t m_ActiveScene;
        std::vector<Scene*> mp_Scenes;
        std::map<std::string, uint32_t> m_SceneIndices;

        bool ShouldSwitchScene = false;
        std::string NextScene;
        SceneChangeCtx NextCtx;

    public:
        SceneManager() = default;

        ErrorType Create(RenderWindow* pWindow);
        void Destroy() const;

        template<typename Scene_t> requires(std::is_base_of_v<Scene, Scene_t>)
        Scene_t* AddScene(const std::string& name);

        template<typename Scene_t> requires(std::is_base_of_v<Scene, Scene_t>)
        Scene_t* GetScene(const std::string& name);

        void SetActiveScene(const std::string& name, SceneChangeCtx& ctx);

        Scene* GetCurrentScene();
        void UpdateCurrentScene();

    };

    template<typename Scene_t> requires (std::is_base_of_v<Scene, Scene_t>)
    Scene_t* SceneManager::AddScene(const std::string& name) {

        Scene_t* pScene = new Scene_t();
        GPC::SceneInformation scene_information{};
        scene_information.pWindow = mp_RenderWindow;
        scene_information.pManager = this;
        pScene->Create(scene_information);

        m_SceneIndices.insert( { name, mp_Scenes.size() } );
        mp_Scenes.push_back(pScene);

        return pScene;
    }

    template<typename Scene_t> requires (std::is_base_of_v<Scene, Scene_t>)
    Scene_t * SceneManager::GetScene(const std::string &name) {
        return static_cast<Scene_t *>(static_cast<void*>(mp_Scenes[m_SceneIndices[name]]));
    }

} // GPC