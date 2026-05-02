#include "Village/VillageController.h"
#include "Village/VillageManager.h"
#include "Village/VillageActions.h"
#include "Village/VillageConditions.h"
#include "Transition.h"
#include "Scene.h"
#include "RenderWindow.h"
#include "UI/UiCanvas.h"

namespace GPC
{
    VillageController::VillageController(VillageManager* pManager) : mp_VillageManager(pManager)
    {
        mp_VillageStateMachine = new StateMachine<VillageController>(this, static_cast<int>(VillageState::COUNT));
        InitializeStateMachine();
    }

    VillageController::~VillageController()
    {
        delete mp_VillageStateMachine;
    }

    void VillageController::Update(Clock& clock)
    {
        mp_VillageStateMachine->Update();

        if (!m_StartInspection && !m_StartTransition && Inputs::IsButtonDown(Inputs::ButtonCode::BUTTON_RIGHT) && m_CanMove) {

            constexpr float CameraSpeed = 12.5f;

            float scale = 1.0f;
            if (Scene* pScene = mp_VillageManager->GetScene())
            {
                if (RenderWindow* pWin = pScene->GetRenderWindow())
                {
                    const float w = pWin->GetSize().x;
                    if (w > 0.0f) scale = w / 1080.0f;
                }
            }

            const float dt = static_cast<float>(clock.GetDeltaTime());
            const float dx = (Inputs::GetDeltaMouseX() / scale) * kCameraDragSensitivity;
            const float dy = (Inputs::GetDeltaMouseY() / scale) * kCameraDragSensitivity;

            glm::vec2 MapSize = mp_VillageManager->GetMap()->GetSize();
            mp_CameraTarget->LocalTransform.AddPosition(CameraSpeed * mp_CameraTarget->LocalTransform.GetRight()   * dt * dx);
            mp_CameraTarget->LocalTransform.AddPosition(CameraSpeed * mp_CameraTarget->LocalTransform.GetForward() * dt * dy);

            if (mp_CameraTarget->LocalTransform.GetPosition().x > MapSize.x)
            {
                mp_CameraTarget->LocalTransform.SetPosition({MapSize.x,
                    mp_CameraTarget->LocalTransform.GetPosition().y,
                    mp_CameraTarget->LocalTransform.GetPosition().z});
            }
            else if (mp_CameraTarget->LocalTransform.GetPosition().x < 0)
            {
                mp_CameraTarget->LocalTransform.SetPosition({0,
                    mp_CameraTarget->LocalTransform.GetPosition().y,
                    mp_CameraTarget->LocalTransform.GetPosition().z});
            }

            if (mp_CameraTarget->LocalTransform.GetPosition().z > MapSize.y)
            {
                mp_CameraTarget->LocalTransform.SetPosition({mp_CameraTarget->LocalTransform.GetPosition().x,
                    mp_CameraTarget->LocalTransform.GetPosition().y,
                    MapSize.y});
            }
            else if (mp_CameraTarget->LocalTransform.GetPosition().z < 0)
            {
                mp_CameraTarget->LocalTransform.SetPosition({mp_CameraTarget->LocalTransform.GetPosition().x,
                    mp_CameraTarget->LocalTransform.GetPosition().y,
                    0});
            }
        }

    }

    void VillageController::SetCamera(Transform3D* cameraTransform, FollowTargetVillage *p_Camera)
    {
        mp_Camera = p_Camera;
        mp_CameraTarget = mp_Camera->pTarget;
        mp_CameraTransform = cameraTransform;

        glm::vec2 MapSize = mp_VillageManager->GetMap()->GetSize();
        mp_CameraTarget->LocalTransform.SetPosition(MapSize.x * 0.5f,  mp_CameraTarget->LocalTransform.GetPosition().y, MapSize.y * 0.5f);
    }

    void VillageController::Start()
    {
        mp_VillageStateMachine->SetState(static_cast<int>(VillageState::VIEW));
    }

    void VillageController::StartBuild()
    {
        mp_Editor->SetPlannerActive(true);
        mp_Editor->GetMap()->SetGridActive(true);
        m_StartBuilding = true;
    }

    void VillageController::EndBuild()
    {
        if (!m_StartBuilding) return;
        mp_Editor->SetPlannerActive(false);
        mp_Editor->GetMap()->SetGridActive(false);
        m_StartBuilding = false;
    }

    bool VillageController::IsBuilding()
    {
        return m_StartBuilding;
    }

    void VillageController::StartInspection()
    {
        m_StartInspection = true;
    }

    void VillageController::EndInspection()
    {
        if (!m_StartInspection) return;
        m_StartInspection = false;
    }

    bool VillageController::IsInspecting()
    {
        return m_StartInspection;
    }

    void VillageController::StartTransition()
    {
        m_StartTransition = true;
        // todo sigsev si on veut enlever UI
        // if (mp_MainUI)    mp_MainUI->IsEnable    = false;
        // if (mp_EditUI)    mp_EditUI->IsEnable    = false;
        // if (mp_InspectUI) mp_InspectUI->IsEnable = false;
    }

    void VillageController::EndTransition()
    {
        if (!m_StartTransition) return;
        m_StartTransition = false;
        // todo sigsev si on veut enlever UI
        // if (mp_MainUI)    mp_MainUI->IsEnable    = true;
        // if (mp_EditUI)    mp_EditUI->IsEnable    = true;
        // if (mp_InspectUI) mp_InspectUI->IsEnable = true;
    }

    bool VillageController::IsTransitioning() const
    {
        return m_StartTransition;
    }

    void VillageController::StartEditing()
    {
        mp_Editor->SetPlannerActive(true);
        mp_Editor->GetMap()->SetGridActive(true);
        m_StartEditing = true;
    }

    void VillageController::EndEditing()
    {
        if (!m_StartEditing) return;
        mp_Editor->SetPlannerActive(false);
        mp_Editor->GetMap()->SetGridActive(false);
        m_StartEditing = false;
    }

    bool VillageController::IsEditing()
    {
        return m_StartEditing;
    }

    void VillageController::StartRoll()
    {
        m_StartRool = true;
    }

    void VillageController::EndRoll()
    {
        m_StartRool = false;
    }

    bool VillageController::IsRolling()
    {
        return m_StartRool;
    }

    bool VillageController::CanMove() const
    {
        return m_CanMove;
    }

    void VillageController::SetCanMove(bool canMove)
    {
        m_CanMove = canMove;
    }


    void VillageController::InitializeStateMachine()
    {
        Behaviour<VillageController>* viewBehaviour = mp_VillageStateMachine->CreateBehaviour(static_cast<int>(VillageState::VIEW));
        Behaviour<VillageController>* editBehaviour = mp_VillageStateMachine->CreateBehaviour(static_cast<int>(VillageState::EDIT));
        Behaviour<VillageController>* inspectBehaviour = mp_VillageStateMachine->CreateBehaviour(static_cast<int>(VillageState::INSPECT));

        // ==========================================
        //      RÈGLES DU MODE VIEW
        // ==========================================
        viewBehaviour->AddAction<ActionToggleScroll>()->Enable = true;
        viewBehaviour->AddAction<ActionToggleMainUI>()->Enable = true;
        viewBehaviour->AddAction<ActionToggleBuildingEdit>()->Enable = false;
        viewBehaviour->AddAction<ActionToggleBuildingInteract>()->Enable = true;
        viewBehaviour->AddAction<ActionToggleEditUI>()->Enable = false;
        viewBehaviour->AddAction<ActionToggleInspectUI>()->Enable = false;

        Transition<VillageController>* viewToEdit = viewBehaviour->CreateTransition(static_cast<int>(VillageState::EDIT));
        viewToEdit->AddCondition<ConditionInputEdit>();

        Transition<VillageController>* viewToInspect = viewBehaviour->CreateTransition(static_cast<int>(VillageState::INSPECT));
        viewToInspect->AddCondition<ConditionInputInspect>();

        // ==========================================
        //      RÈGLES DU MODE EDIT
        // ==========================================
        editBehaviour->AddAction<ActionToggleScroll>()->Enable = true;
        editBehaviour->AddAction<ActionToggleMainUI>()->Enable = false;
        editBehaviour->AddAction<ActionToggleBuildingEdit>()->Enable = true;
        editBehaviour->AddAction<ActionToggleEditUI>()->Enable = true;
        editBehaviour->AddAction<ActionToggleBuildingInteract>()->Enable = false;
        editBehaviour->AddAction<ActionToggleInspectUI>()->Enable = false;

        Transition<VillageController>* editToView = editBehaviour->CreateTransition(static_cast<int>(VillageState::VIEW));
        editToView->AddCondition<ConditionInputBack>();

        // ==========================================
        //      RÈGLES DU MODE INSPECT
        // ==========================================
        inspectBehaviour->AddAction<ActionToggleScroll>()->Enable = false;
        inspectBehaviour->AddAction<ActionToggleMainUI>()->Enable = false;
        inspectBehaviour->AddAction<ActionToggleBuildingEdit>()->Enable = false;
        inspectBehaviour->AddAction<ActionToggleEditUI>()->Enable = false;
        inspectBehaviour->AddAction<ActionToggleBuildingInteract>()->Enable = false;
        inspectBehaviour->AddAction<ActionToggleInspectUI>()->Enable = true;

        Transition<VillageController>* inspectToView = inspectBehaviour->CreateTransition(static_cast<int>(VillageState::VIEW));
        inspectToView->AddCondition<ConditionInputBack>();
    }
}