#pragma once
#include <cstdint>

#include "BuildingManager.h"
#include "FollowTarget.h"
#include "FollowTargetVillage.h"
#include "GPC_Framework.h"
#include "StateMachine.h"

namespace GPC
{
    class VillageManager;
    struct FreeCamera;
    class Editor;
    struct UiCanvas;

    enum class VillageState : int
    {
        VIEW = 0,
        EDIT,
        INSPECT,
        COUNT
    };

    class VillageController
    {
    public:
        VillageController(VillageManager* p_Manager);
        ~VillageController();

        void                                Start();
        void                                Update(Clock& clock);
        [[nodiscard]] VillageManager*       GetVillageManager() const { return  mp_VillageManager; }

        [[nodiscard]] Editor*               GetEditor() const { return mp_Editor; }
        void                                SetEditor(Editor* p_Editor) { mp_Editor = p_Editor; }

        [[nodiscard]] FollowTargetVillage*  GetCamera() const { return mp_Camera; }
        void                                SetCamera(Transform3D* cameraTransform, FollowTargetVillage* p_Camera);

        [[nodiscard]] UiCanvas*             GetMainUI() const { return mp_MainUI; }
        void                                SetMainUI(UiCanvas* p_UI) { mp_MainUI = p_UI; }

        [[nodiscard]] UiCanvas*             GetEditUI() const { return mp_EditUI; }
        void                                SetEditUI(UiCanvas* p_UI) { mp_EditUI = p_UI; }

        [[nodiscard]] UiCanvas*             GetInspectUI() const { return mp_InspectUI; }
        void                                SetInspectUI(UiCanvas* p_UI) { mp_InspectUI = p_UI; }

        void                                StartBuild();
        void                                EndBuild();
        bool                                IsBuilding();

        void                                StartInspection();
        void                                EndInspection();
        bool                                IsInspecting();

        void                                StartTransition();
        void                                EndTransition();
        bool                                IsTransitioning() const;

        void                                StartEditing();
        void                                EndEditing();
        bool                                IsEditing();

        void                                StartRoll();
        void                                EndRoll();
        bool                                IsRolling();

        bool                                CanMove() const;
        void                                SetCanMove(bool canMove);

        static constexpr float              kCameraDragSensitivity = 0.1f;

    private:
        void                                InitializeStateMachine();

        StateMachine<VillageController>*    mp_VillageStateMachine;
        VillageManager* mp_VillageManager;

        FollowTargetVillage*   mp_Camera    = nullptr;
        Transform3D*    mp_CameraTransform  = nullptr;
        Transform3D*    mp_CameraTarget     = nullptr;

        Editor*         mp_Editor           = nullptr;

        UiCanvas*       mp_MainUI           = nullptr;
        UiCanvas*       mp_EditUI           = nullptr;
        UiCanvas*       mp_InspectUI        = nullptr;

        bool m_StartEditing     =  false;
        bool m_StartInspection  =  false;
        bool m_StartBuilding    =  false;
        bool m_StartRool        =  false;
        bool m_StartTransition  =  false;
        bool m_CanMove          =  true;
    };
}