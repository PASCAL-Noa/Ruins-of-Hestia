#pragma once
#include "Behavior.h"
#include "Dialogue/DialogueBehaviour.h"
#include "Village/VillageController.h"
#include "Village/VillageUiController.h"
#include "Village/Buildings/BuildingBehavior.h"

namespace GPC {

    namespace TutoQueue
    {
        bool  IsTutoActive();
        float LastTutoEndTime();
        bool  CanSpawnTuto(float currentTime, float minGap = 1.0f);

        void  NotifyTutoStarted();
        void  NotifyTutoEnded(float endTime);
    }

    struct ExpeditionMovementTuto : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(ExpeditionMovementTuto);

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        float Start_time = 0.0f;

    private:
        EntityID m_DialogueDisplay              = 0;
        DialogueBehaviour* mp_DialogueBehavior = nullptr;
        Clock* mp_Clock                         = nullptr;
    };

    struct ExpeditionHarvestTuto : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(ExpeditionHarvestTuto);

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

    private:
        EntityID m_DialogueDisplay              = 0;
        DialogueBehaviour* mp_DialogueBehavior = nullptr;
        Clock* mp_Clock                         = nullptr;
    };

    struct ExpeditionCombatTuto : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(ExpeditionCombatTuto);

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

    private:
        EntityID m_DialogueDisplay              = 0;
        DialogueBehaviour* mp_DialogueBehavior = nullptr;
        Clock* mp_Clock                         = nullptr;
    };

    struct ExpeditionEndOfMissionTuto : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(ExpeditionEndOfMissionTuto);

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

    private:
        EntityID m_DialogueDisplay              = 0;
        DialogueBehaviour* mp_DialogueBehavior = nullptr;
        Clock* mp_Clock                         = nullptr;
    };

    struct TutoDialogue
    {
        DialogueBehaviour* pCurrent = nullptr;
        TutoDialogue*      pNext    = nullptr;
        glm::vec2          position{};
        bool               isRight = false;

        std::string        filePath;
        bool               autoClose = true;
        Anchor              anchor   = Anchors::TOP_LEFT;
    };

    struct VillageTuto : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(VillageTuto);

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void InitVillageTuto(BuildingBehavior* pBarn, BuildingBehavior* pLifeTree, BuildingBehavior* pForge, VillageController* pController, VillageUiController* pUiController, bool* pIsInTuto);
        void IsGoingToExpedition();
    private:
        void ShowSilene(glm::vec2 pos, bool isRight, Anchor anchor);
        void HideSilene();
        void UpdateSilenePos();

        void InitBarnDialogue();

        //TUTO BARN
        void StartBarnTuto();
        void LaunchCurrentDialogue();

        void StartLifeTreeTuto();
        void StartBuildingTuto();
        void StartForgeTuto();
        void StartExpeditionTuto();

        void InitForgeTuto();

        void HandleBarn();
        void HandleTree();
        void HandleBuild();
        void HandleForge();
        void HandleExpedition();

    private:
        const BehaviorCreateContext* mp_Ctx = nullptr;
        Clock* mp_Clock = nullptr;
        UiCanvas* mp_Canvas = nullptr;
        UiSprite* mp_SileneRight = nullptr;
        UiSprite* mp_SileneLeft = nullptr;
        UiSprite* mp_CurrentSilene = nullptr;

        BuildingBehavior* mp_Barn;
        BuildingBehavior* mp_Forge;
        BuildingBehavior* mp_LifeTree;

        VillageController* mp_VillageController = nullptr;
        VillageUiController* mp_VillageUiController = nullptr;


        //BEFORE INTERACT
        EntityID m_DialogueDisplay              = 0;
        DialogueBehaviour* mp_DialogueBehavior   = nullptr;
        glm::vec2   m_SilenePos;
        EntityID    m_Silene;


        TutoDialogue*  mp_CurrentDialogue       = nullptr;
        TutoDialogue*  mp_NameDialogue          = nullptr;
        TutoDialogue*  mp_UpgradeDialogue       = nullptr;
        TutoDialogue*  mp_CycleDialogue         = nullptr;
        TutoDialogue*  mp_DescriptionDialogue   = nullptr;

        // BUILD TUTO
        TutoDialogue*  mp_BuildButtonDialogue   = nullptr;
        TutoDialogue*  mp_BuildPriceDialogue   = nullptr;
        TutoDialogue*  mp_ResourcesDialogue   = nullptr;
        TutoDialogue*  mp_BuildDescDialogue   = nullptr;
        TutoDialogue*  mp_BuildHouseDialogue   = nullptr;

        // FORGE TUTO
        TutoDialogue*  mp_ForgeDialogue                     = nullptr;
        TutoDialogue*  mp_ForgeOreDialogue                  = nullptr;
        TutoDialogue*  mp_ForgeWeaponDialogue               = nullptr;
        TutoDialogue*  mp_ForgeWeaponRerollMenuDialogue     = nullptr;
        TutoDialogue*  mp_ForgeWeaponRerollBlocDialogue     = nullptr;


        // EXPE TUTO
        TutoDialogue*  mp_ExpeButtonDialogue     = nullptr;

        bool* mp_IsInTuto = nullptr;

        // If Of Death
        bool m_IsBarnTutorialBegin       = false;
        bool m_IsBarnTutorialEnded       = false;

        bool m_IsLifeTutorialBegin       = false;
        bool m_IsLifeTutorialEnded       = false;

        bool m_IsBuildTutorialBegin       = false;
        bool m_IsBuildTutorialEnded       = false;
        bool m_HasBuiltHouse              = false;

        bool m_IsForgeTutorialBegin       = false;
        bool m_IsForgeTutorialEnded       = false;

        bool m_IsExpeditionTutorialBegin  = false;
        bool m_IsExpeditionTutorialEnded  = false;

        float m_AnimSileneDuration        = 1.0f;
        glm::vec2 m_AnimSileneDeltaPos    = {0.f, 10.f};
    };


    struct LaunchExpeditionTuto : public Behavior
    {
          INHERIT_BEHAVIOR_CONSTRUCTOR(LaunchExpeditionTuto);

        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void Init(UiButton* Zone1Button, UiButton* ExpeditionButton, UiText* endlessText, bool* pIsInTutorial);


    private:
        void ShowSilene(glm::vec2 pos, bool isRight, Anchor anchor);
        void HideSilene();
        void UpdateSilenePos();

        void InitAllDialogs();

        void HandleBiome();
        void StartBiomeTuto();

        void HandlePrep();
        void StartPrepTuto();

        void LaunchCurrentDialogue();

    private:
        const BehaviorCreateContext* mp_Ctx = nullptr;
        Clock* mp_Clock = nullptr;
        UiCanvas* mp_Canvas = nullptr;
        UiSprite* mp_SileneRight = nullptr;
        UiSprite* mp_SileneLeft = nullptr;
        UiSprite* mp_CurrentSilene = nullptr;


        //BEFORE INTERACT
        EntityID m_DialogueDisplay              = 0;
        DialogueBehaviour* mp_DialogueBehavior   = nullptr;
        glm::vec2   m_SilenePos;
        EntityID    m_Silene;


        TutoDialogue*  mp_CurrentDialogue               = nullptr;

        // BIOME TUTO
        TutoDialogue* mp_TutoBiomeDialogue              = nullptr;
        TutoDialogue* mp_TutoResourceBiomeDialogue      = nullptr;
        TutoDialogue* mp_TutoZoneSelectionDialogue      = nullptr;
        TutoDialogue* mp_TutoNormalMissionDialogue      = nullptr;
        TutoDialogue* mp_TutoEndlessMissionDialogue     = nullptr;
        TutoDialogue* mp_TutoLaunchZoneDialogue         = nullptr;

        // PREP TUTO
        TutoDialogue* mp_TutoTutoPrepInventaireDialogue = nullptr;
        TutoDialogue* mp_TutoCharactersDialogue         = nullptr;
        TutoDialogue* mp_TutoEquipementsDialogue        = nullptr;
        TutoDialogue* mp_TutoLaunchExpeditionDialogue   = nullptr;


        UiButton*   mp_Zone1Button        = nullptr;
        UiButton*   mp_ExpeditionButton   = nullptr;
        UiText*     mp_EndlessText          = nullptr;

        bool* mp_IsInTuto           = nullptr;

        bool m_IsInit               = false;

        bool m_IsBiomeTutoStarted   = false;
        bool m_IsBiomeTutoEnded     = false;

        bool m_IsPrepTutoStarted    = false;
        bool m_IsPrepTutoEnded      = false;

        float m_AnimSileneDuration        = 1.0f;
        glm::vec2 m_AnimSileneDeltaPos    = {0.f, 10.f};
    };



} // GPC