#pragma once

#include <memory>

#include "AnimationComponent.h"
#include "SceneDefault.h"
#include "Village/BuildingData.h"
#include "Village/Inventory.h"
#include "Village/Map/Map.h"
#include "AudioSystem.h"
#include "FbxLoader.h"
#include "UISettingsGame_Behavior.h"
#include "Expedition/TutoBehaviors.h"
#include "Village/Buildings/BuildingBehavior.h"
#include "TransiFeuillle.h"

namespace GPC
{
    class Editor;
    struct DialogueBehaviour;
    class BuildingManager;
    class VillageManager;
    class VillageController;
    class VillageUiController;
    struct UiCanvas;
    struct UiText;
    struct UiSprite;

    struct SceneExpeditionStartCtx;

    struct SceneVillageStartCtx {
        Inventory*               pExpeditionInventory = nullptr;
        SceneExpeditionStartCtx* pPendingExpedition   = nullptr;
    };

    struct ResourceCountsSnapshot
    {
        uint32_t Fiber = 0;
        uint32_t Wood  = 0;
        uint32_t Food  = 0;
        uint32_t Stone = 0;
        uint32_t Ore   = 0;
    };

    class SceneVillageV1 : public SceneDefault
    {
        CameraComponent*    mp_FreeCameraCamera = nullptr;
        FreeCamera*         mp_FreeCameraBehavior = nullptr;
        bool                m_FreeCamera = false;

    public:

        SceneVillageV1();
        ~SceneVillageV1() override;

        ErrorType OnAssetsLoad() override;
        ErrorType OnCreate(SceneInformation& info) override;
        ErrorType OnStart(SceneChangeCtx &ctx) override;
        void OnSceneUpdate() override;
        void OnDestroy() override;

        VillageManager* GetVillageManager() const { return m_VillageManager.get(); }

        void BeginTuto();
        void CreateBaseVillage();

    private:

        bool m_IsFirst                          = true;

        std::unique_ptr<BuildingManager>        m_BuildingManager;
        std::unique_ptr<VillageManager>         m_VillageManager;
        std::unique_ptr<VillageController>      m_VillageController;
        std::unique_ptr<VillageUiController>    m_UiController;
        std::unique_ptr<Map>                    m_Map;

        BuildingBehavior* mp_BaseLifeTree               = nullptr;
        BuildingBehavior* mp_BaseBarn                   = nullptr;
        BuildingBehavior* mp_BaseHouse                  = nullptr;
        BuildingBehavior* mp_BaseForge                  = nullptr;
        VillageTuto*      mp_VillageTutoBehaviour       = nullptr;

        UiCanvas*         mp_LaunchingScreen            = nullptr;
        UiSprite*         mp_LaunchingBackground        = nullptr;
        UiText*           mp_LaunchingText              = nullptr;
        UiCanvas*         mp_RecapPanel                 = nullptr;
        TransiFeuillle*   mp_ReturnTransition           = nullptr;
        UiText*           mp_RecapFiberText             = nullptr;
        UiText*           mp_RecapWoodText              = nullptr;
        UiText*           mp_RecapFoodText              = nullptr;
        UiText*           mp_RecapStoneText             = nullptr;
        UiText*           mp_RecapOreText               = nullptr;
        BuildingBehavior* mp_LifeTreeBehavior           = nullptr;
        EntityID          m_TreePulseTweenEntity        = 0;
        glm::vec3         m_LifeTreeBaseScale           = { 0.0f, 0.0f, 0.0f };
        glm::vec3         m_LifeTreeBasePos             = { 0.0f, 0.0f, 0.0f };
        bool              m_LifeTreeBaseScaleCaptured   = false;

        ResourceCountsSnapshot m_ProductionDelta        = {};
        bool                   m_RecapPending           = false;
        bool                   m_TreePulseActive        = false;
        bool                   m_LaunchInProgress       = false;
        bool                   m_LaunchAwaitingCamFocus = false;
        SceneExpeditionStartCtx* mp_PendingLaunchLoadout = nullptr;

        FbxMeshData* mp_SileneMeshData;
        AnimationComponent* mp_Animator;

        EntityID m_PlayerEntity{};
        EntityID m_EditorEntity{};

        Editor* mp_Editor = nullptr;

        UISettings* mp_SettingsBehaviour = nullptr;

        Transform3D* mp_ControllerTarget = nullptr;
        DialogueBehaviour*  mp_DialogueBehaviour    = nullptr;

        AudioSource* mp_VillageMusic    = nullptr;
        AudioSource* mp_VillageNature   = nullptr;
        AudioSource* mp_VillageIntro    = nullptr;

        bool m_IsInTutorial           = false;

        void CreateCamera();
        void SyncVillageMap() const;

        void DebugInputs();
        void SetupCallBacks();
        void SetupController();
        void SetupVisuals();
        void SetupAudio();

        void SetupDialogues();
        void OnExpeditionRequested();
        void OnBuildingRequested(BuildingType requested);

        void LoadTutoElements();
        void LoadUIElements();
        void LoadAnimations();
        void LoadParticles();

        void                    SetupSilene();

        void                    SetupLaunchingScreenUI();
        void                    SetupRecapPanelUI();
        void                    SetupReturnTransition();
        void                    PlayLaunchExpeditionSequence(SceneExpeditionStartCtx* pLoadout);
        void                    PushLaunchPostFocusSlots();
        void                    PlayReturnRevealSequence();
        void                    FocusCameraOnLifeTree();
        void                    OpenRecapPanel(const ResourceCountsSnapshot& delta);
        void                    CloseRecapPanel();
        void                    StartTreePulse();
        void                    StopTreePulse();
        void                    FindLifeTreeBehavior();
        void                    SetHierarchyEnabled(EntityID root, bool enabled);
        ResourceCountsSnapshot  CaptureResourceCounts();
        void                    ResetSequenceState();

    };

}
