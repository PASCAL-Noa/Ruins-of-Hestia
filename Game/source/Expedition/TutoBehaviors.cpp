#include "Expedition/TutoBehaviors.h"

#include "Scene.h"
#include "Dialogue/DialogueBehaviour.h"
#include "UI/UiAnimFluent.h"
#include "UI/UiBuilder.h"
#include "UI/UiCanvas.h"
#include "Village/VillageUiController.h"

namespace GPC {

    namespace TutoQueue
    {
        static bool  s_Active          = false;
        static float s_LastTutoEndTime = -1000.0f;

        bool  IsTutoActive()                                  { return s_Active; }
        float LastTutoEndTime()                               { return s_LastTutoEndTime; }
        bool  CanSpawnTuto(float currentTime, float minGap)   { return !s_Active && currentTime > s_LastTutoEndTime + minGap; }

        void  NotifyTutoStarted()                             { s_Active = true; }
        void  NotifyTutoEnded(float endTime)                  { s_Active = false; s_LastTutoEndTime = endTime; }
    }

    static void StartTutoDialogue(const BehaviorCreateContext* pCtx,
                                  EntityID& outEntity,
                                  DialogueBehaviour*& outBehavior,
                                  Clock*& outClock,
                                  const std::string& fileName)
    {
        pCtx->pClock->SetTimeScale(0.0);
        outClock = pCtx->pClock;

        outEntity   = pCtx->pScene->CreateEntityAs2D();
        outBehavior = pCtx->pScene->AddBehavior<DialogueBehaviour>(outEntity);

        std::string commonPath = "Resources/DialoguesText/Tuto/Expedition/";
        commonPath += fileName;

        outBehavior->StartDialogue(commonPath, std::vector<UiCanvas*>{});

        TutoQueue::NotifyTutoStarted();
    }

    static void TryEndTuto(const BehaviorUpdateContext* pCtx,
                           DialogueBehaviour* behavior,
                           Clock* clock,
                           EntityID dialogueEntity,
                           EntityID selfEntity)
    {
        if (behavior != nullptr && behavior->IsDialogueEnded())
        {
            if (clock) clock->SetTimeScale(1.0);
            TutoQueue::NotifyTutoEnded(static_cast<float>(pCtx->pClock->GetTime()));
            pCtx->pScene->DestroyEntity(dialogueEntity);
            pCtx->pScene->DestroyEntity(selfEntity);
        }
    }

    void ExpeditionMovementTuto::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);
        StartTutoDialogue(pCtx, m_DialogueDisplay, mp_DialogueBehavior, mp_Clock, "TutoMovement.txt");
    }

    void ExpeditionMovementTuto::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);
        TryEndTuto(pCtx, mp_DialogueBehavior, mp_Clock, m_DialogueDisplay, GetEntityID());
    }

    void ExpeditionHarvestTuto::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);
        StartTutoDialogue(pCtx, m_DialogueDisplay, mp_DialogueBehavior, mp_Clock, "TutoCollect.txt");
    }

    void ExpeditionHarvestTuto::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);
        TryEndTuto(pCtx, mp_DialogueBehavior, mp_Clock, m_DialogueDisplay, GetEntityID());
    }

    void ExpeditionCombatTuto::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);
        StartTutoDialogue(pCtx, m_DialogueDisplay, mp_DialogueBehavior, mp_Clock, "TutoAttack.txt");
    }

    void ExpeditionCombatTuto::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);
        TryEndTuto(pCtx, mp_DialogueBehavior, mp_Clock, m_DialogueDisplay, GetEntityID());
    }

    void ExpeditionEndOfMissionTuto::OnStart(const BehaviorCreateContext *pCtx) {
        Behavior::OnStart(pCtx);
        StartTutoDialogue(pCtx, m_DialogueDisplay, mp_DialogueBehavior, mp_Clock, "TutoFireCamp.txt");
    }

    void ExpeditionEndOfMissionTuto::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);
        TryEndTuto(pCtx, mp_DialogueBehavior, mp_Clock, m_DialogueDisplay, GetEntityID());
    }

    void VillageTuto::OnStart(const BehaviorCreateContext* pCtx)
    {
        Behavior::OnStart(pCtx);
        mp_Ctx = pCtx;

        mp_CurrentDialogue = new TutoDialogue();

        m_DialogueDisplay = pCtx->pScene->CreateEntityAs2D();

        mp_DialogueBehavior = pCtx->pScene->AddBehavior<DialogueBehaviour>(m_DialogueDisplay);

        mp_DialogueBehavior->StartDialogue("Resources/DialoguesText/Tuto/Village/TutoFerme.txt",
            {}, false);

        UiBuilder b(pCtx->pScene);
        mp_Canvas    = b.At(Anchors::TOP_LEFT, {0.0f, 0.0f})
                            .Tint(Colors::TRANSPARENT)
                            .FitToScreen(true)
                            .BuildCanvas();

        mp_SileneRight = b.ChildOf(mp_Canvas->GetEntityID())
                            .At(Anchors::TOP_LEFT, {0.0f, 0.0f})
                            .Scale({ 0.28f, 0.28f })
                            .Sprite("NerdSilene", "Nerd_Silene_Right")
                            .Tint(Colors::TRANSPARENT)
                            .BuildSprite();

        mp_SileneLeft = b.ChildOf(mp_Canvas->GetEntityID())
                            .At(Anchors::TOP_LEFT, { 0.0f, 0.0f})
                            .Scale({ 0.28f, 0.28f })
                            .Sprite("NerdSilene", "Nerd_Silene_Left")
                            .Tint(Colors::TRANSPARENT)
                            .BuildSprite();

        mp_VillageController->SetCanMove(false);

        mp_VillageUiController->GetBuildingButton()->IsEnable = false;
        mp_VillageUiController->GetEditorButton()->IsEnable = false;
        mp_VillageUiController->GetSettingsButton()->IsEnable = false;
        mp_VillageUiController->GetExpeditionButton()->IsEnable = false;
        InitBarnDialogue();

    }

    void VillageTuto::OnUpdate(const BehaviorUpdateContext* pCtx)
    {
        Behavior::OnUpdate(pCtx);

        UpdateSilenePos();

        HandleBarn();

        HandleTree();

        HandleBuild();

        HandleForge();

        HandleExpedition();
    }

    void VillageTuto::InitVillageTuto(BuildingBehavior* pBarn, BuildingBehavior* pLifeTree, BuildingBehavior* pForge, VillageController* pController, VillageUiController* pUiController, bool* pIsInTuto)
    {
        mp_Barn = pBarn;
        mp_Forge = pForge;
        mp_LifeTree = pLifeTree;
        mp_VillageController = pController;
        mp_VillageUiController = pUiController;
        mp_IsInTuto = pIsInTuto;
    }

    void VillageTuto::IsGoingToExpedition()
    {
        if (m_IsExpeditionTutorialBegin && mp_DialogueBehavior != nullptr)
        {
            mp_DialogueBehavior->StopDialog();
        }
    }

    void VillageTuto::ShowSilene(const glm::vec2 pos, const bool isRight, Anchor anchor)
    {
        HideSilene();

        if (isRight)
        {
            mp_CurrentSilene = mp_SileneRight;
        }
        else
        {
            mp_CurrentSilene = mp_SileneLeft;
        }

        mp_CurrentSilene->SetTint(Colors::WHITE);
        mp_CurrentSilene->Anchor_ = anchor;
        mp_Ctx->pScene->GetComponent<Transform2D>(mp_CurrentSilene->GetEntityID())->LocalTransform.SetPosition({pos.x, pos.y, 0.0f});
    }

    void VillageTuto::HideSilene()
    {
        if (mp_CurrentSilene)
        {
            mp_CurrentSilene->SetTint(Colors::TRANSPARENT);
            mp_CurrentSilene = nullptr;
        }
    }

    void VillageTuto::UpdateSilenePos()
    {
        if (mp_CurrentSilene == nullptr)
            return;

        Transform2D* Transform = mp_Ctx->pScene->GetComponent<Transform2D>(mp_CurrentSilene->GetEntityID());

        float elapsed = mp_Ctx->pClock->GetDeltaTime();
        float ratio = elapsed / m_AnimSileneDuration;

        glm::vec2 basePos = mp_CurrentDialogue->position;

        if (Transform->LocalTransform.GetPosition().y > basePos.y + abs(m_AnimSileneDeltaPos.y))
        {
            m_AnimSileneDeltaPos.y = -abs(m_AnimSileneDeltaPos.y);
        }
        else if (Transform->LocalTransform.GetPosition().y < basePos.y - abs(m_AnimSileneDeltaPos.y))
        {
            m_AnimSileneDeltaPos.y = abs(m_AnimSileneDeltaPos.y);
        }

        glm::vec3 add = glm::vec3{m_AnimSileneDeltaPos.x, m_AnimSileneDeltaPos.y, 0.f} * ratio;

        glm::vec3 NextPos = Transform->LocalTransform.GetPosition() + add;


        Transform->LocalTransform.SetPosition(NextPos.x, NextPos.y, 0.0f);
    }

    void VillageTuto::InitBarnDialogue()
    {
        mp_NameDialogue      = new TutoDialogue();
        mp_UpgradeDialogue   = new TutoDialogue();
        mp_CycleDialogue     = new TutoDialogue();
        mp_DescriptionDialogue = new TutoDialogue();
        mp_CurrentDialogue   = nullptr;

        mp_NameDialogue->filePath           = "Resources/DialoguesText/Tuto/Village/TutoName.txt";
        mp_UpgradeDialogue->filePath        = "Resources/DialoguesText/Tuto/Village/TutoUpgrade.txt";
        mp_CycleDialogue->filePath          = "Resources/DialoguesText/Tuto/Village/TutoCycle.txt";
        mp_DescriptionDialogue->filePath    = "Resources/DialoguesText/Tuto/Village/TutoDescription.txt";

        mp_NameDialogue->position      = {462.f, 183.f};
        mp_UpgradeDialogue->position   = {400.f, 324.f};
        mp_CycleDialogue->position     = {40.f, 223.f};
        mp_DescriptionDialogue->position = {60.f, 480.f};

        mp_NameDialogue->isRight      = false;
        mp_UpgradeDialogue->isRight   = false;
        mp_CycleDialogue->isRight     = true;
        mp_DescriptionDialogue->isRight     = true;

        mp_NameDialogue->pNext      = mp_UpgradeDialogue;
        mp_UpgradeDialogue->pNext   = mp_CycleDialogue;
        mp_CycleDialogue->pNext     = mp_DescriptionDialogue;
        mp_DescriptionDialogue->pNext = nullptr;





        mp_BuildButtonDialogue  = new TutoDialogue();
        mp_BuildPriceDialogue   = new TutoDialogue();
        mp_ResourcesDialogue    = new TutoDialogue();
        mp_BuildDescDialogue    = new TutoDialogue();
        mp_BuildHouseDialogue   = new TutoDialogue();

        mp_BuildButtonDialogue->filePath    = "Resources/DialoguesText/Tuto/Village/TutoBuildButton.txt";
        mp_BuildPriceDialogue->filePath    = "Resources/DialoguesText/Tuto/Village/TutoBuildPrice.txt";
        mp_ResourcesDialogue->filePath    = "Resources/DialoguesText/Tuto/Village/TutoResources.txt";
        mp_BuildDescDialogue->filePath    = "Resources/DialoguesText/Tuto/Village/TutoBuildDesc.txt";
        mp_BuildHouseDialogue->filePath    = "Resources/DialoguesText/Tuto/Village/TutoBuildHouse.txt";

        mp_BuildButtonDialogue->position = {120.f, 630.f};
        mp_BuildPriceDialogue->position = {222.f, 202.f};
        mp_ResourcesDialogue->position = {300.f, 84.f};
        mp_BuildDescDialogue->position = {290.f, 190.f};
        mp_BuildHouseDialogue->position = {222.f, 202.f};

        mp_BuildButtonDialogue->isRight    = false;
        mp_BuildPriceDialogue->isRight     = false;
        mp_ResourcesDialogue->isRight      = false;
        mp_BuildDescDialogue->isRight      = false;
        mp_BuildHouseDialogue->isRight     = false;

        mp_BuildButtonDialogue->autoClose   = false;
        mp_BuildHouseDialogue->autoClose    = false;

        mp_BuildButtonDialogue->pNext   = mp_BuildPriceDialogue;
        mp_BuildPriceDialogue->pNext    = mp_ResourcesDialogue;
        mp_ResourcesDialogue ->pNext    = mp_BuildDescDialogue;
        mp_BuildDescDialogue ->pNext    = mp_BuildHouseDialogue;
        mp_BuildHouseDialogue->pNext    = nullptr;



        mp_ForgeDialogue                                    = new TutoDialogue();
        mp_ForgeOreDialogue                                 = new TutoDialogue();
        mp_ForgeWeaponDialogue                              = new TutoDialogue();
        mp_ForgeWeaponRerollMenuDialogue                    = new TutoDialogue();
        mp_ForgeWeaponRerollBlocDialogue                    = new TutoDialogue();

        mp_ForgeDialogue                  ->filePath        = "Resources/DialoguesText/Tuto/Village/TutoForge.txt";
        mp_ForgeOreDialogue               ->filePath        = "Resources/DialoguesText/Tuto/Village/TutoForgeOre.txt";
        mp_ForgeWeaponDialogue            ->filePath        = "Resources/DialoguesText/Tuto/Village/TutoForgeWeapon.txt";
        mp_ForgeWeaponRerollMenuDialogue  ->filePath        = "Resources/DialoguesText/Tuto/Village/TutoForgeWeaponRerollBloc.txt";
        mp_ForgeWeaponRerollBlocDialogue  ->filePath        = "Resources/DialoguesText/Tuto/Village/TutoForgeWeaponReroll.txt";

        mp_ForgeDialogue                  ->position     = {120,-60};
        mp_ForgeOreDialogue               ->position     = {66.f, 526.f};
        mp_ForgeWeaponDialogue            ->position     = {178.f, 262.f};
        mp_ForgeWeaponRerollMenuDialogue  ->position     = {-125.0f, -30.0f};
        mp_ForgeWeaponRerollBlocDialogue  ->position     = {-25.0f, 180.0f};

        mp_ForgeDialogue                  ->isRight         = false;
        mp_ForgeOreDialogue               ->isRight         = false;
        mp_ForgeWeaponDialogue            ->isRight         = false;
        mp_ForgeWeaponRerollMenuDialogue  ->isRight         = false;
        mp_ForgeWeaponRerollBlocDialogue  ->isRight         = false;

        mp_ForgeDialogue                  ->autoClose       = false;
        //mp_ForgeWeaponDialogue            ->autoClose       = false;

        mp_ForgeDialogue                  ->pNext           = mp_ForgeOreDialogue;
        mp_ForgeOreDialogue               ->pNext           = mp_ForgeWeaponDialogue;
        mp_ForgeWeaponDialogue            ->pNext           = mp_ForgeWeaponRerollMenuDialogue;
        mp_ForgeWeaponRerollBlocDialogue  ->pNext           = nullptr;
        mp_ForgeWeaponRerollMenuDialogue  ->pNext           = mp_ForgeWeaponRerollBlocDialogue;

        mp_ForgeDialogue                  ->anchor          = Anchors::MIDDLE_MIDDLE;
        mp_ForgeWeaponRerollMenuDialogue  ->anchor          = Anchors::MIDDLE_RIGHT;
        mp_ForgeWeaponRerollBlocDialogue  ->anchor          = Anchors::MIDDLE_RIGHT;


        mp_ExpeButtonDialogue                                   = new TutoDialogue();
        mp_ExpeButtonDialogue                   ->filePath      = "Resources/DialoguesText/Tuto/Village/TutoExpeButton.txt";
        mp_ExpeButtonDialogue                   ->position      = {-120.f, -20.f};
        mp_ExpeButtonDialogue                   ->isRight          = true;
        mp_ExpeButtonDialogue                   ->autoClose        = false;
        mp_ExpeButtonDialogue                   ->pNext            = nullptr;
        mp_ExpeButtonDialogue                   ->anchor           = Anchors::BOTTOM_RIGHT;

    }

    void VillageTuto::StartBarnTuto()
    {
        mp_CurrentDialogue = mp_NameDialogue;
        LaunchCurrentDialogue();
    }

    void VillageTuto::LaunchCurrentDialogue()
    {
        if (!mp_CurrentDialogue)
            return;

        ShowSilene(mp_CurrentDialogue->position, mp_CurrentDialogue->isRight, mp_CurrentDialogue->anchor);

        mp_DialogueBehavior->StartDialogue(mp_CurrentDialogue->filePath, {}, mp_CurrentDialogue->autoClose);
    }

    void VillageTuto::StartLifeTreeTuto()
    {
        m_IsLifeTutorialBegin = true;
        mp_DialogueBehavior->StartDialogue("Resources/DialoguesText/Tuto/Village/TutoLifeTree.txt", {}, true);
    }

    void VillageTuto::StartBuildingTuto()
    {
        mp_VillageUiController->GetBuildingButton()->IsEnable = true;
        mp_CurrentDialogue = mp_BuildButtonDialogue;
        LaunchCurrentDialogue();
    }

    void VillageTuto::StartForgeTuto()
    {
        mp_CurrentDialogue = mp_ForgeOreDialogue;
        LaunchCurrentDialogue();
    }

    void VillageTuto::StartExpeditionTuto()
    {
        mp_VillageUiController->GetExpeditionButton()->IsEnable = true;
        m_IsExpeditionTutorialBegin = true;
        mp_CurrentDialogue = mp_ExpeButtonDialogue;
        LaunchCurrentDialogue();
    }

    void VillageTuto::InitForgeTuto()
    {
        mp_Forge->Focus({-4.0f, -4.0f});
        mp_CurrentDialogue = mp_ForgeDialogue;
        LaunchCurrentDialogue();
    }

    void VillageTuto::HandleBarn()
    {
        if (mp_Barn->IsInteracting && !m_IsBarnTutorialBegin && !m_IsBarnTutorialEnded)
        {
            mp_DialogueBehavior->StopDialog();
            m_IsBarnTutorialBegin = true;

            StartBarnTuto();
        }

        if (m_IsBarnTutorialBegin && !m_IsBarnTutorialEnded && mp_CurrentDialogue && mp_DialogueBehavior->IsDialogueEnded())
        {

            mp_CurrentDialogue = mp_CurrentDialogue->pNext;

            if (mp_CurrentDialogue)
            {
                LaunchCurrentDialogue();
            }
            else
            {
                HideSilene();
                m_IsBarnTutorialEnded = true;


                mp_Barn->CloseOpened();
                mp_VillageController->EndInspection();
            }
        }
    }

    void VillageTuto::HandleTree()
    {
        if (!m_IsLifeTutorialBegin && !m_IsLifeTutorialEnded && m_IsBarnTutorialEnded)
        {
            mp_VillageController->StartInspection();
            mp_LifeTree->Focus({-4.0f, -4.0f});
            StartLifeTreeTuto();
        }
        if (m_IsLifeTutorialBegin && !m_IsLifeTutorialEnded && mp_DialogueBehavior->IsDialogueEnded())
        {
            mp_VillageController->EndInspection();
            m_IsLifeTutorialEnded = true;
            StartBuildingTuto();
        }
    }

    void VillageTuto::HandleBuild()
    {
        if (mp_VillageUiController->GetState() == VillageUiState::Buildings && m_IsLifeTutorialEnded && !m_IsBuildTutorialBegin)
        {
            mp_DialogueBehavior->StopDialog();
            m_IsBuildTutorialBegin = true;
        }

        if (m_IsBuildTutorialBegin && !m_IsBuildTutorialEnded && mp_VillageController->IsBuilding())
        {
            mp_DialogueBehavior->StopDialog();
        }

        if (m_IsBuildTutorialEnded && mp_VillageController->IsBuilding() == false && m_HasBuiltHouse == false)
        {
            mp_VillageController->EndBuild();
            InitForgeTuto();
            m_HasBuiltHouse = true;
            mp_VillageController->SetCanMove(false);
        }

        if (m_IsBuildTutorialBegin && !m_IsBuildTutorialEnded && mp_CurrentDialogue && mp_DialogueBehavior->IsDialogueEnded())
        {
            mp_CurrentDialogue = mp_CurrentDialogue->pNext;

            if (mp_CurrentDialogue)
            {
                LaunchCurrentDialogue();
            }
            else
            {
                HideSilene();
                m_IsBuildTutorialEnded = true;
                mp_VillageController->SetCanMove(true);
            }

        }
    }

    void VillageTuto::HandleForge()
    {
        if (mp_Forge->IsInteracting && !m_IsForgeTutorialBegin && !m_IsForgeTutorialEnded)
        {
            mp_DialogueBehavior->StopDialog();
            m_IsForgeTutorialBegin = true;

            StartForgeTuto();
        }

        if (mp_VillageController->IsRolling() && m_IsForgeTutorialBegin && !m_IsForgeTutorialEnded)
        {
            mp_DialogueBehavior->StopDialog();
        }

        if (m_IsForgeTutorialBegin && !m_IsForgeTutorialEnded && mp_CurrentDialogue && mp_DialogueBehavior->IsDialogueEnded())
        {
            mp_CurrentDialogue = mp_CurrentDialogue->pNext;

            if (mp_CurrentDialogue)
            {
                LaunchCurrentDialogue();
            }
            else
            {
                HideSilene();
                m_IsForgeTutorialEnded = true;

                mp_Forge->CloseOpened();
                mp_VillageController->EndRoll();
                mp_VillageController->EndInspection();


                StartExpeditionTuto();
            }
        }
    }

    void VillageTuto::HandleExpedition()
    {
        if (m_IsExpeditionTutorialBegin && !m_IsExpeditionTutorialEnded && mp_CurrentDialogue && mp_DialogueBehavior->IsDialogueEnded())
        {
            mp_CurrentDialogue = mp_CurrentDialogue->pNext;

            if (mp_CurrentDialogue)
            {
                LaunchCurrentDialogue();
            }
            else
            {
                HideSilene();
                m_IsExpeditionTutorialEnded = true;
                mp_VillageController->SetCanMove(true);
                mp_VillageUiController->GetEditorButton()->IsEnable = true;
                mp_VillageUiController->GetSettingsButton()->IsEnable = true;

                // mp_Ctx->pScene->DestroyEntity(mp_DialogueBehavior->GetEntityID());
                // mp_Ctx->pScene->DestroyEntity(GetEntityID());

                mp_DialogueBehavior         = nullptr;
                *mp_IsInTuto = false;
            }
        }
    }

    void LaunchExpeditionTuto::OnStart(const BehaviorCreateContext* pCtx)
    {
        Behavior::OnStart(pCtx);
        mp_Ctx = pCtx;

        mp_CurrentDialogue = new TutoDialogue();

        m_DialogueDisplay = pCtx->pScene->CreateEntityAs2D();

        mp_DialogueBehavior = pCtx->pScene->AddBehavior<DialogueBehaviour>(m_DialogueDisplay);

        UiBuilder b(pCtx->pScene);
        mp_Canvas    = b.At(Anchors::TOP_LEFT, {0.0f, 0.0f})
                            .Tint(Colors::TRANSPARENT)
                            .FitToScreen(true)
                            .BuildCanvas();

        mp_SileneRight = b.ChildOf(mp_Canvas->GetEntityID())
                            .At(Anchors::TOP_LEFT, {0.0f, 0.0f})
                            .Scale({ 0.28f, 0.28f })
                            .Sprite("NerdSilene", "Nerd_Silene_Right")
                            .Tint(Colors::TRANSPARENT)
                            .BuildSprite();

        mp_SileneLeft = b.ChildOf(mp_Canvas->GetEntityID())
                            .At(Anchors::TOP_LEFT, { 0.0f, 0.0f})
                            .Scale({ 0.28f, 0.28f })
                            .Sprite("NerdSilene", "Nerd_Silene_Left")
                            .Tint(Colors::TRANSPARENT)
                            .BuildSprite();

        InitAllDialogs();
    }

    void LaunchExpeditionTuto::OnUpdate(const BehaviorUpdateContext* pCtx)
    {
        Behavior::OnUpdate(pCtx);

        if (m_IsInit == false)
            return;;

        UpdateSilenePos();

        HandleBiome();

        HandlePrep();
    }

    void LaunchExpeditionTuto::Init(UiButton* Zone1Button, UiButton* ExpeditionButton, UiText* endlessText,
        bool* pIsInTutorial)
    {
        mp_Zone1Button = Zone1Button;
        mp_ExpeditionButton = ExpeditionButton;
        mp_EndlessText = endlessText;
        mp_IsInTuto = pIsInTutorial;
    }


    void LaunchExpeditionTuto::ShowSilene(glm::vec2 pos, bool isRight, Anchor anchor)
    {
        HideSilene();

        if (isRight)
        {
            mp_CurrentSilene = mp_SileneRight;
        }
        else
        {
            mp_CurrentSilene = mp_SileneLeft;
        }

        mp_CurrentSilene->SetTint(Colors::WHITE);
        mp_CurrentSilene->Anchor_ = anchor;
        mp_Ctx->pScene->GetComponent<Transform2D>(mp_CurrentSilene->GetEntityID())->LocalTransform.SetPosition({pos.x, pos.y, 0.0f});
    }

    void LaunchExpeditionTuto::HideSilene()
    {
        if (mp_CurrentSilene)
        {
            mp_CurrentSilene->SetTint(Colors::TRANSPARENT);
            mp_CurrentSilene = nullptr;
        }
    }

    void LaunchExpeditionTuto::UpdateSilenePos()
    {
        if (mp_CurrentSilene == nullptr)
            return;

        Transform2D* Transform = mp_Ctx->pScene->GetComponent<Transform2D>(mp_CurrentSilene->GetEntityID());

        float elapsed = mp_Ctx->pClock->GetDeltaTime();
        float ratio = elapsed / m_AnimSileneDuration;

        glm::vec2 basePos = mp_CurrentDialogue->position;

        if (Transform->LocalTransform.GetPosition().y > basePos.y + abs(m_AnimSileneDeltaPos.y))
        {
            m_AnimSileneDeltaPos.y = -abs(m_AnimSileneDeltaPos.y);
        }
        else if (Transform->LocalTransform.GetPosition().y < basePos.y - abs(m_AnimSileneDeltaPos.y))
        {
            m_AnimSileneDeltaPos.y = abs(m_AnimSileneDeltaPos.y);
        }

        glm::vec3 add = glm::vec3{m_AnimSileneDeltaPos.x, m_AnimSileneDeltaPos.y, 0.f} * ratio;

        glm::vec3 NextPos = Transform->LocalTransform.GetPosition() + add;


        Transform->LocalTransform.SetPosition(NextPos.x, NextPos.y, 0.0f);
    }

    void LaunchExpeditionTuto::InitAllDialogs()
    {
        mp_CurrentDialogue = nullptr;

        /// BEFORE INTERACT
        mp_TutoBiomeDialogue                = new TutoDialogue();
        mp_TutoResourceBiomeDialogue        = new TutoDialogue();
        mp_TutoZoneSelectionDialogue        = new TutoDialogue();
        mp_TutoNormalMissionDialogue        = new TutoDialogue();
        mp_TutoEndlessMissionDialogue       = new TutoDialogue();
        mp_TutoLaunchZoneDialogue           = new TutoDialogue();

        mp_TutoTutoPrepInventaireDialogue   = new TutoDialogue();
        mp_TutoEquipementsDialogue          = new TutoDialogue();
        mp_TutoCharactersDialogue           = new TutoDialogue();
        mp_TutoLaunchExpeditionDialogue     = new TutoDialogue();

        /// BEFORE INTERACT
        mp_TutoBiomeDialogue                ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoBiome.txt";
        mp_TutoResourceBiomeDialogue        ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoResourceBiome.txt";
        mp_TutoZoneSelectionDialogue        ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoZoneSelectionBiome.txt";
        mp_TutoNormalMissionDialogue        ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoNormalMission.txt";
        mp_TutoEndlessMissionDialogue       ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoEndlessMission.txt";
        mp_TutoLaunchZoneDialogue           ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoLaunchZone.txt";

        mp_TutoTutoPrepInventaireDialogue   ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoPrepInventaire.txt";
        mp_TutoEquipementsDialogue          ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoEquipements.txt";
        mp_TutoCharactersDialogue           ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoCharacters.txt";
        mp_TutoLaunchExpeditionDialogue     ->filePath    = "Resources/DialoguesText/Tuto/PrepExpedition/TutoLaunchExpedition.txt";

        /// BEFORE INTERACT
        mp_TutoBiomeDialogue                ->position =            {68, 534};
        mp_TutoResourceBiomeDialogue        ->position =            {68, 534};
        mp_TutoZoneSelectionDialogue        ->position =            {930, 380};
        mp_TutoNormalMissionDialogue        ->position =            {-100, -50};
        mp_TutoEndlessMissionDialogue       ->position =            {-50, -40};
        mp_TutoLaunchZoneDialogue           ->position =            {-100, -50};

        mp_TutoNormalMissionDialogue->anchor    =  Anchors::MIDDLE_RIGHT;
        mp_TutoEndlessMissionDialogue->anchor   = Anchors::BOTTOM_RIGHT;
        mp_TutoLaunchZoneDialogue->anchor       = Anchors::MIDDLE_RIGHT;

        mp_TutoTutoPrepInventaireDialogue   ->position =            {80, -100};
        mp_TutoEquipementsDialogue          ->position =            {95, 420};
        mp_TutoCharactersDialogue           ->position =            {55, 650};
        mp_TutoLaunchExpeditionDialogue     ->position =            {-120, 0};

        mp_TutoTutoPrepInventaireDialogue->anchor = Anchors::MIDDLE_MIDDLE;
        mp_TutoLaunchExpeditionDialogue->anchor = Anchors::BOTTOM_RIGHT;

        /// BEFORE INTERACT
        mp_TutoBiomeDialogue                ->isRight =  true;
        mp_TutoResourceBiomeDialogue        ->isRight =  true;
        mp_TutoZoneSelectionDialogue        ->isRight =  true;
        mp_TutoNormalMissionDialogue        ->isRight  = false;
        mp_TutoEndlessMissionDialogue       ->isRight =  false;
        mp_TutoLaunchZoneDialogue           ->isRight =  false;

        mp_TutoTutoPrepInventaireDialogue   ->isRight    =  true;
        mp_TutoEquipementsDialogue          ->isRight    =  true;
        mp_TutoCharactersDialogue           ->isRight    =  true;
        mp_TutoLaunchExpeditionDialogue     ->isRight    =  true;

        /// BEFORE INTERACT
        mp_TutoBiomeDialogue                ->pNext = mp_TutoResourceBiomeDialogue;
        mp_TutoResourceBiomeDialogue        ->pNext = mp_TutoZoneSelectionDialogue;
        mp_TutoZoneSelectionDialogue        ->pNext = mp_TutoNormalMissionDialogue;
        mp_TutoNormalMissionDialogue        ->pNext = mp_TutoEndlessMissionDialogue;
        mp_TutoEndlessMissionDialogue       ->pNext = mp_TutoLaunchZoneDialogue;
        mp_TutoLaunchZoneDialogue           ->pNext = nullptr;

        mp_TutoTutoPrepInventaireDialogue   ->pNext   = mp_TutoEquipementsDialogue;
        mp_TutoEquipementsDialogue          ->pNext   = mp_TutoCharactersDialogue;
        mp_TutoCharactersDialogue           ->pNext   = mp_TutoLaunchExpeditionDialogue;
        mp_TutoLaunchExpeditionDialogue     ->pNext   = nullptr;

        /// BEFORE INTERACT
        mp_TutoLaunchZoneDialogue           ->autoClose = false;
        mp_TutoLaunchExpeditionDialogue     ->autoClose = false;

        m_IsInit = true;
    }

    void LaunchExpeditionTuto::HandleBiome()
    {
        if (!m_IsBiomeTutoStarted && !m_IsBiomeTutoEnded)
        {
            m_IsBiomeTutoStarted = true;

            StartBiomeTuto();
        }

        if (m_IsBiomeTutoStarted && !m_IsBiomeTutoEnded && mp_CurrentDialogue && mp_DialogueBehavior->IsDialogueEnded())
        {
            mp_CurrentDialogue = mp_CurrentDialogue->pNext;

            if (mp_CurrentDialogue)
            {
                LaunchCurrentDialogue();
            }
            else
            {
                HideSilene();
            }
        }
    }

    void LaunchExpeditionTuto::StartBiomeTuto()
    {
        mp_CurrentDialogue = mp_TutoBiomeDialogue;
        LaunchCurrentDialogue();
    }

    void LaunchExpeditionTuto::HandlePrep()
    {
        if (m_IsPrepTutoStarted == false && mp_Zone1Button->IsClicked == true)
        {
            mp_DialogueBehavior->StopDialog();
            m_IsBiomeTutoEnded = true;
            m_IsPrepTutoStarted = true;

            mp_EndlessText->SetText("ENDLESS");

            StartPrepTuto();
        }

        if (m_IsPrepTutoStarted && mp_ExpeditionButton->IsClicked)
        {
            HideSilene();
            if (mp_DialogueBehavior)
            {
                mp_DialogueBehavior->StopDialog();
                mp_DialogueBehavior->SetVisibility(false);
                mp_DialogueBehavior = nullptr;
            }
            m_IsPrepTutoEnded = true;
            mp_DialogueBehavior = nullptr;
            *mp_IsInTuto = false;
        }

        if (m_IsPrepTutoStarted && !m_IsPrepTutoEnded && mp_CurrentDialogue && mp_DialogueBehavior->IsDialogueEnded())
        {
            mp_CurrentDialogue = mp_CurrentDialogue->pNext;

            if (mp_CurrentDialogue)
            {
                LaunchCurrentDialogue();
            }
            else
            {
                HideSilene();
                m_IsPrepTutoEnded = true;
                mp_DialogueBehavior->SetVisibility(false);
                mp_DialogueBehavior = nullptr;
                *mp_IsInTuto = false;
            }
        }
    }

    void LaunchExpeditionTuto::StartPrepTuto()
    {
        mp_CurrentDialogue = mp_TutoTutoPrepInventaireDialogue;
        LaunchCurrentDialogue();
    }

    void LaunchExpeditionTuto::LaunchCurrentDialogue()
    {
        if (!mp_CurrentDialogue)
            return;

        ShowSilene(mp_CurrentDialogue->position, mp_CurrentDialogue->isRight, mp_CurrentDialogue->anchor);

        mp_DialogueBehavior->StartDialogue(mp_CurrentDialogue->filePath, {}, mp_CurrentDialogue->autoClose);
    }
} // GPC