#include "Scenes/SceneVillageV1.h"

#include <cmath>

#include "AudioListener.h"
#include "FollowTarget.h"
#include "FollowTargetVillage.h"
#include "Scenes/SceneExemple.h"
#include "FreeCamera.h"
#include "LightingSystem.h"
#include "Render3DSystem.h"
#include "TransformComponents.h"

#include "Village/BuildingManager.h"
#include "Village/VillageManager.h"
#include "Village/VillageController.h"
#include "Village/VillageUiController.h"
#include "Village/Map/Map.h"
#include "Village/Map/Editor.h"
#include "Village/Buildings/LifeTree/LifeTreeBehavior.h"

#include "Inputs.h"
#include "ParticleSystem.h"
#include "SceneManager.h"
#include "SerialFile.h"
#include "SerialTree.h"
#include "TweenEffects.h"
#include "UiConstants.h"
#include "UI/UiCanvas.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"
#include "UI/UiButton.h"
#include "UI/UiBuilder.h"
#include "UI/UiAnim.h"
#include "UI/UiAnimFluent.h"
#include "Dialogue/DialogueBehaviour.h"
#include "Scenes/SceneExpedition.h"
#include "Scenes/SceneExpeditionPrep.h"

#include "SettingsGame.h"
#include "Expedition/TutoBehaviors.h"
#include "Scenes/SceneMainMenu.h"
#include "Village/Save.h"

static bool m_HasTutoStart = false;

namespace
{
    struct BuildingUIData
    {
        std::string Icon;
        std::string Title;
        std::string Description;
        GPC::BuildingType Type;
        bool IsImplemented;
    };

    const std::vector<BuildingUIData> BUILDING_CATALOG = {
        { "Icon_Habitat", "HABITATION", "Une batisse simple", GPC::BuildingType::House, true },
        { "Icon_Ferme", "FERME", "NOA", GPC::BuildingType::Farm, true },
        { "Icon_Forge", "FORGE", "TRISTAN", GPC::BuildingType::Forge, true },
        { "Icon_Comming_Soon", "HOPITAL", "ETHAN", GPC::BuildingType::Hospital, false },
        { "Icon_Comming_Soon", "CASERNE", "TIAGO", GPC::BuildingType::Count, false },
        { "Icon_Comming_Soon", "DECORATION", "THOMAS", GPC::BuildingType::Count, false }
    };
}

namespace GPC
{
    SceneVillageV1::SceneVillageV1()  = default;
    SceneVillageV1::~SceneVillageV1() = default;

    ErrorType SceneVillageV1::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();

        mp_SileneMeshData = GPC_FBX->Load("Silene", "Resources/Models/Characters/Silene/Idle_silene.fbx");

        SetCollisionGridScale({70, 70});

        LoadTutoElements();
        LoadUIElements();
        LoadAnimations();
        LoadParticles();

        // mp_AttackAudio->Path = "Resources/Audio/SFX/sfx.wav";
        // mp_AttackAudio->Resource = GPC_AUDIO("Shoot");
        // mp_AttackAudio->TypeVoice = AudioType::SFX;
        // mp_AttackAudio->Volume = 0.5f;
        // mp_AttackAudio->Looping = false;

        SettingsGame::Get().Load();

        return ErrorType::SUCCESS;
    }

    ErrorType SceneVillageV1::OnCreate(SceneInformation &info)
    {
        SceneDefault::OnCreate(info);

        CreateDebugOverlay();

        m_VillageManager  = std::make_unique<VillageManager>(this);
        m_BuildingManager = std::make_unique<BuildingManager>(this, m_VillageManager.get());
        m_Map             = std::make_unique<Map>(this, glm::vec2(70.0f, 70.0f));
        m_VillageManager->SetMap(m_Map.get());

        auto settingsEntity = CreateEntityAs2D();
        mp_SettingsBehaviour = AddBehavior<UISettings>(settingsEntity);

        return ErrorType::SUCCESS;
    }

    ErrorType SceneVillageV1::OnStart(SceneChangeCtx &ctx) {
        const ErrorType ret = SceneDefault::OnStart(ctx);

        CtxMainMenuToVillage mainMenuCtx = {};
        if (ctx.DataSize == sizeof(CtxMainMenuToVillage))
        {
            memcpy(&mainMenuCtx, &ctx.pData, sizeof(CtxMainMenuToVillage));
        }

        ResetSequenceState();

        SceneVillageStartCtx* pCtx = ctx.pData ? static_cast<SceneVillageStartCtx*>(ctx.pData) : nullptr;

        if (ctx.DataSize == sizeof(SceneVillageStartCtx))
        {
            if (pCtx && pCtx->pPendingExpedition && m_VillageManager) {
                PlayLaunchExpeditionSequence(pCtx->pPendingExpedition);
                pCtx->pPendingExpedition = nullptr;
                return ret;
            }
        }


        if (m_IsFirst)
        {
            SetupVisuals();
            SetupCallBacks();

            m_VillageManager->LoadVillageData();
            m_VillageManager->GiveDefaultEquipments(); // TODO On save pas les equirments dcp bon
            SyncVillageMap();

            SetupAudio();

            if (mainMenuCtx.isContinue == false)
            {
                SerialTree tree;
                if (SerialFile::Load(Saves::SAVE_FILE_PATH, tree) != ErrorType::SUCCESS && !m_HasTutoStart)
                {
                    CreateBaseVillage();
                    m_IsInTutorial = true;
                    mp_VillageIntro->Play();
                }
            }

            CreateCamera();

            SetupController();

            SetupLaunchingScreenUI();
            SetupRecapPanelUI();
            SetupReturnTransition();
            SetupSilene();

            m_IsFirst = false;
            return ErrorType::SUCCESS;
        }

        FindLifeTreeBehavior();

        if (m_VillageManager) {
            m_VillageManager->AdvanceTime();
        }

        if (m_VillageManager)
        {
            const bool isReturning = (pCtx && pCtx->pExpeditionInventory != nullptr);

            if (!isReturning)
            {
                m_VillageManager->SetAllBuildingsActive(false);
                m_VillageManager->SyncBuildingActiveState();
                m_VillageManager->SaveVillageData();
            }
            else
            {
                m_VillageManager->GetInventory().AddAllFromInventory(*pCtx->pExpeditionInventory);

                ResourceCountsSnapshot before = CaptureResourceCounts();
                m_VillageManager->AdvanceTime();
                ResourceCountsSnapshot after = CaptureResourceCounts();

                m_ProductionDelta.Fiber = after.Fiber - before.Fiber;
                m_ProductionDelta.Wood  = after.Wood  - before.Wood;
                m_ProductionDelta.Food  = after.Food  - before.Food;
                m_ProductionDelta.Stone = after.Stone - before.Stone;
                m_ProductionDelta.Ore   = after.Ore   - before.Ore;

                if (mp_LifeTreeBehavior)
                {
                    mp_LifeTreeBehavior->OnRecapInteract = [this]()
                    {
                        if (m_RecapPending) OpenRecapPanel(m_ProductionDelta);
                    };
                }
                PlayReturnRevealSequence();
            }
        }

        return ret;
    }

    void SceneVillageV1::OnSceneUpdate()
    {
        SceneDefault::OnSceneUpdate();

        DebugInputs();
        if (m_UiController) m_UiController->Update();

        if (mp_DialogueBehaviour && m_UiController && mp_DialogueBehaviour->IsDialogueEnded()
            && m_UiController->GetState() == VillageUiState::Dialogue)
        {
            m_UiController->LeaveDialogue();
        }

        if (m_IsInTutorial && !m_HasTutoStart)
        {
            BeginTuto();
        }

        if (m_LaunchAwaitingCamFocus && mp_MainCameraTransform && mp_MainCameraBehaviour && mp_MainCameraBehaviour->pTarget)
        {
            const glm::vec3 camPos  = mp_MainCameraTransform->LocalTransform.GetPosition();
            const glm::vec3 wantPos = mp_MainCameraBehaviour->pTarget->LocalTransform.GetPosition() + mp_MainCameraBehaviour->Offset;
            const float dist = glm::length(camPos - wantPos);

            constexpr float kCamFocusThreshold = 0.5f;
            if (dist < kCamFocusThreshold)
            {
                m_LaunchAwaitingCamFocus = false;
                PushLaunchPostFocusSlots();
            }
        }

        if (!m_LaunchInProgress)
        {
            if (m_VillageController) m_VillageController->Update(m_Clock);
            if (m_VillageManager)    m_VillageManager->Update();
            m_VillageManager->UpdatePlacement(this, m_VillageController.get(), m_Clock.GetDeltaTime());
        }

        if (m_TreePulseActive && m_LifeTreeBaseScaleCaptured && mp_LifeTreeBehavior && mp_LifeTreeBehavior->Render3DComponent_)
        {
            EntityID spriteId = mp_LifeTreeBehavior->Render3DComponent_->GetEntityID();
            Transform3D* tr = GetComponent<Transform3D>(spriteId);
            if (tr)
            {
                const float t = m_Clock.GetTime();
                const float pulseAmount = 0.06f * std::sin(t * 3.0f);
                const float pulse = 1.0f + pulseAmount;

                tr->LocalTransform.SetScale(m_LifeTreeBaseScale * pulse);

                const float heightDelta = m_LifeTreeBaseScale.z * pulseAmount;
                tr->LocalTransform.SetPosition(m_LifeTreeBasePos + glm::vec3(0.0f, heightDelta * 0.5f, 0.0f));
            }
        }

        if (!mp_VillageIntro->IsPlaying() && !mp_VillageMusic->IsPlaying()) {
            mp_VillageMusic->Play();
            mp_VillageNature->Play();
        }

    }

    void SceneVillageV1::OnDestroy()
    {
        m_UiController.reset();
        m_VillageController.reset();
        m_Map.reset();
        m_BuildingManager.reset();
        m_VillageManager.reset();
    }

    void SceneVillageV1::CreateCamera()
    {
        auto tagetId = CreateEntityAs3D();
        mp_ControllerTarget = GetComponent<Transform3D>(tagetId);
        AddComponent<AudioListener>(tagetId);
        mp_ControllerTarget->LocalTransform.RotateYawPitchRoll(glm::radians(45.0f), 0.0f, 0.0f);

        constexpr float CAMERA_DISTANCE = 100.0f * GENERAL_SCALE_VILLAGE;
        mp_MainCamera->UseOrthographic(10);

        mp_MainCameraTransform->LocalTransform.SetRotationYawPitchRoll({glm::radians(225.0f), glm::radians(-39.5f), 0});
        mp_MainCameraTransform->LocalTransform.SetPosition(mp_MainCameraTransform->LocalTransform.GetForward() * CAMERA_DISTANCE);

        mp_MainCameraBehaviour = AddBehavior<FollowTargetVillage>(mp_MainCamera->GetEntityID());
        mp_MainCameraBehaviour->LerpSpeed = 8.0f;
        mp_MainCameraBehaviour->pTarget = mp_ControllerTarget;
        mp_MainCameraBehaviour->Offset = mp_MainCameraTransform->LocalTransform.GetForward() * CAMERA_DISTANCE + glm::vec3( 3.0f, 0.0f, 3.0f );

        // auto camera_libre = CreateEntityAs3D();
        // auto clTransform    = GetComponent<Transform3D>(camera_libre);
        // clTransform->LocalTransform.SetPosition(0.0f, 50.0f, 0.0f);
        // clTransform->LocalTransform.SetRotationYawPitchRoll(0, glm::radians(-45.0f), 0);
        // mp_FreeCameraCamera = AddComponent<CameraComponent>(camera_libre);
        // mp_FreeCameraBehavior = AddBehavior<FreeCamera>(camera_libre);
        // mp_FreeCameraBehavior->MovementSpeed = 30.0f * GENERAL_SCALE_EXPEDITION;

    }

    void SceneVillageV1::SyncVillageMap() const
    {
        m_BuildingManager->BuildingCleaner();
        m_Map->Clear();

        for (int i = 0; i < m_VillageManager->GetBuildingsData().size(); i++) {
            auto& building = m_VillageManager->GetBuildingsData()[i];
            if (building.Type >= BuildingType::Count) continue;
            BuildingBehavior* behavior = nullptr;
            EntityID id = m_BuildingManager->SpawnBuildingFromType(building.Type, building.Position, &behavior);
            m_Map->RegisterBuilding(building.Position, behavior->Building_->GetSize(), id);
            m_VillageManager->GetBuildingBehavior().push_back(behavior);
            building.LinkedBehavior = behavior;
        }
    }

    void SceneVillageV1::SetupCallBacks()
    {
        m_VillageManager->OnBuildingAddedCallback = [this](BuildingType type, glm::ivec2 pos)
        {
            BuildingBehavior* behavior = nullptr;
            EntityID id = m_BuildingManager->SpawnBuildingFromType(type, pos, &behavior);
            m_Map->RegisterBuilding(pos, behavior->Building_->GetSize(), id);
            if (m_VillageManager->GetAudioSources().size() > 0)
                m_VillageManager->GetAudioSources()[0]->Play();
            return behavior;
        };

        m_VillageManager->OnBuildingStashedCallback = [this](glm::ivec2 pos, BuildingType type)
        {
            if (EntityID id = m_Map->GetEntityAt(pos))
            {
                m_Map->RemovePlacedZone(id);
                m_BuildingManager->StashEntity(id, type);
            }
        };

        m_VillageManager->OnBuildingUnstashedCallback = [this](BuildingBehavior* type, glm::ivec2 pos)
        {
            m_Map->RegisterBuilding(pos, type->Building_->GetSize(), type->GetEntityID());
        };
    }

    void SceneVillageV1::SetupController()
    {
        m_EditorEntity = CreateEntityAs3D();

        auto* renderer = AddComponent<Render3DComponent>(m_EditorEntity);
        renderer->pGeometry = GPC_MESH("Plane");
        renderer->RenderMaterial.Tint = Colors::GREEN;
        renderer->RenderMaterial.EmissiveStrength = 1.0f;

        auto* transform = GetComponent<Transform3D>(m_EditorEntity);
        transform->LocalTransform.SetScale(glm::vec3(1.0f));

        mp_Editor = AddBehavior<Editor>(m_EditorEntity);
        mp_Editor->SetMap(m_Map.get());
        mp_Editor->SetCursor(mp_ControllerTarget);
        mp_Editor->SetVillageManager(m_VillageManager.get());

        m_VillageController = std::make_unique<VillageController>(m_VillageManager.get());

        m_VillageController->SetCamera(mp_MainCameraTransform, mp_MainCameraBehaviour);
        m_VillageController->SetEditor(mp_Editor);

        mp_SettingsBehaviour->OnSettingsChanged.ConnectLambda([this]() {
            if (mp_VillageMusic) {
                mp_VillageMusic->Volume = SettingsGame::Get().GetVolume(AudioType::Music);
                mp_VillageNature->Volume = SettingsGame::Get().GetVolume(AudioType::Music);
                mp_VillageIntro ->Volume = SettingsGame::Get().GetVolume(AudioType::Music);
            }

            float sfxVol = SettingsGame::Get().GetVolume(AudioType::SFX);
            for (AudioSource* sfx : m_VillageManager->GetAudioSources()) {
                if (sfx) sfx->Volume = sfxVol;
            }
            SettingsGame::Get().Save();
        });

        mp_SettingsBehaviour->OnCloseRequested.ConnectLambda([this]()
        {
            m_UiController->SetState(VillageUiState::Idle);
            // TODO : mp_UIpopClose->Play();
        });

        m_UiController = std::make_unique<VillageUiController>(this, m_VillageManager.get(), m_BuildingManager.get(), mp_SettingsBehaviour);
        m_UiController->Build();
        m_VillageController->SetMainUI(m_UiController->GetMainCanvas());

        m_UiController->OnExpeditionRequested.ConnectLambda([this]()
        {
            if (mp_VillageTutoBehaviour != nullptr)
            {
                mp_VillageTutoBehaviour->IsGoingToExpedition();
            }

            OnExpeditionRequested();
        });
        m_UiController->OnBuildingRequested.ConnectLambda([this](BuildingType requested)
        {
            OnBuildingRequested(requested);
        });

        SetupDialogues();
        m_VillageController->Start();

        m_VillageManager->SetVillageController(m_VillageController.get());
    }

    void SceneVillageV1::OnExpeditionRequested()
    {
        auto* pPrep = new SceneExpeditionPrepCtx();
        if (m_VillageManager) pPrep->pVillageInventory = &m_VillageManager->GetInventory();

        SceneChangeCtx ctx{};
        ctx.pData    = pPrep;
        ctx.DataSize = sizeof(SceneExpeditionPrepCtx);

        SwapToScene("ExpeditionPrep", ctx);
    }

    void SceneVillageV1::OnBuildingRequested(BuildingType requested)
    {
        m_VillageManager->StartPlacement(requested);
    }

    void SceneVillageV1::CreateBaseVillage()
    {
        mp_BaseLifeTree = m_VillageManager.get()->AddBuilding(BuildingType::LifeTree, {35,35});

        mp_BaseBarn = m_VillageManager.get()->AddBuilding(BuildingType::Farm, {45,25});
        m_VillageManager.get()->AddBuilding(BuildingType::Farm, {45,38});

        mp_BaseForge = m_VillageManager.get()->AddBuilding(BuildingType::Forge, {20,20});

        //m_VillageManager.get()->AddBuilding(BuildingType::House, {37,45});
        m_VillageManager.get()->AddBuilding(BuildingType::House, {25,35});
        mp_BaseHouse = m_VillageManager.get()->AddBuilding(BuildingType::House, {25,42});


    }

    void SceneVillageV1::BeginTuto()
    {

        mp_BaseBarn->Focus({-2.0f, -2.0f});

        const auto ent = CreateEntityAs2D();

        mp_VillageTutoBehaviour = AddBehavior<VillageTuto>(ent);
        mp_VillageTutoBehaviour->InitVillageTuto(mp_BaseBarn, mp_BaseLifeTree, mp_BaseForge, m_VillageController.get(), m_UiController.get(), &m_IsInTutorial);

        m_HasTutoStart = true;
    }

    void SceneVillageV1::DebugInputs()
    {
#ifdef NDEBUG
        return;
#endif

        if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::G))
        {
            TweenEffects::Spin(
                GetComponent<TweenComponent>(m_PlayerEntity),
                GetComponent<Transform3D>(m_PlayerEntity)
            );
        }

        if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::H))
        {
            TweenEffects::Shake(
                GetComponent<TweenComponent>(mp_MainCamera->GetEntityID()),
                GetComponent<Transform3D>(mp_MainCamera->GetEntityID())
            );
        }

        if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::TAB))
        {
            uint32_t wood = m_VillageManager->GetInventory().GetResourceCount<Wood<ResourceTier::T1>>();
            uint32_t food = m_VillageManager->GetInventory().GetResourceCount<Food>();

            GPC_INFO << "Wood: " << wood << " | Food: " << food << ENDL;
            GPC_INFO << "Bagnards en stock : " << m_VillageManager->GetStashedCount(BuildingType::Farm) << ENDL;
            GPC_INFO << "Forges en stock : " << m_VillageManager->GetStashedCount(BuildingType::Forge) << ENDL;
        }

        if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::U))
        {
            m_VillageManager->GetInventory().AddResource<Wood<ResourceTier::T1>>(9999);
            m_VillageManager->GetInventory().AddResource<Stone<ResourceTier::T1>>(9999);
            m_VillageManager->GetInventory().AddResource<Metal<ResourceTier::T1>>(9999);
            m_VillageManager->GetInventory().AddResource<Fiber<ResourceTier::T1>>(9999);
            m_VillageManager->GetInventory().AddResource<Food>(9999);
            m_VillageManager->GetInventory().AddResource<GreenGemme>(9999);
            m_VillageManager->GetInventory().AddResource<RedGemme>(9999);
            m_VillageManager->GetInventory().AddResource<BlueGemme>(9999);
            m_VillageManager->GetInventory().AddResource<OrangeGemme>(9999);
            m_VillageManager->GetInventory().AddResource<PurpleGemme>(9999);
            m_VillageManager->GetInventory().AddResource<YellowGemme>(9999);


            auto* debugGloves = new Gloves(1);
            debugGloves->OnBaseStatistics();
            m_VillageManager->GetInventory().AddEquipment(debugGloves);

            auto* debugAxe = new Axe(1);
            debugAxe->OnBaseStatistics();
            m_VillageManager->GetInventory().AddEquipment(debugAxe);

            // auto* debugChestPlate = new ChestPlate(1);
            // debugChestPlate->OnBaseStatistics();
            // m_VillageManager->GetInventory().AddEquipment(debugChestPlate);
            //
            // auto* debugHelmet = new Helmet(1);
            // debugHelmet->OnBaseStatistics();
            // m_VillageManager->GetInventory().AddEquipment(debugHelmet);
            //
            // auto* debugLegs = new Legs(1);
            // debugLegs->OnBaseStatistics();
            // m_VillageManager->GetInventory().AddEquipment(debugLegs);
            //
            // auto* debugBoots = new Boots(1);
            // debugBoots->OnBaseStatistics();
            // m_VillageManager->GetInventory().AddEquipment(debugBoots);
            //
            // auto* debugLongSword = new LongSword(1);
            // debugLongSword->OnBaseStatistics();
            // m_VillageManager->GetInventory().AddEquipment(debugLongSword);
            //
            // auto* debugTomahawk = new Tomahawk(1);
            // debugTomahawk->OnBaseStatistics();
            // m_VillageManager->GetInventory().AddEquipment(debugTomahawk);
            //
            // auto* debugBow = new Bow(1);
            // debugBow->OnBaseStatistics();
            // m_VillageManager->GetInventory().AddEquipment(debugBow);
        }

        if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::O))
        {
            GPC_INFO << "=== INVENTAIRE DEBUG ===" << ENDL;

            uint32_t wood = m_VillageManager->GetInventory().GetResourceCount<Wood<ResourceTier::T1>>();
            uint32_t stone = m_VillageManager->GetInventory().GetResourceCount<Stone<ResourceTier::T1>>();
            uint32_t metal = m_VillageManager->GetInventory().GetResourceCount<Metal<ResourceTier::T1>>();
            uint32_t fiber = m_VillageManager->GetInventory().GetResourceCount<Fiber<ResourceTier::T1>>();
            uint32_t food = m_VillageManager->GetInventory().GetResourceCount<Food>();

            GPC_INFO << "Wood: " << wood << " | Stone: " << stone << " | Metal: " << metal << " | Fiber: " << fiber << " | Food: " << food << ENDL;

            uint32_t gGem = m_VillageManager->GetInventory().GetResourceCount<GreenGemme>();
            uint32_t rGem = m_VillageManager->GetInventory().GetResourceCount<RedGemme>();
            uint32_t bGem = m_VillageManager->GetInventory().GetResourceCount<BlueGemme>();
            uint32_t oGem = m_VillageManager->GetInventory().GetResourceCount<OrangeGemme>();
            uint32_t pGem = m_VillageManager->GetInventory().GetResourceCount<PurpleGemme>();
            uint32_t yGem = m_VillageManager->GetInventory().GetResourceCount<YellowGemme>();

            GPC_INFO << "Green: " << gGem << " | Red: " << rGem << " | Blue: " << bGem << " | Orange: " << oGem << " | Purple: " << pGem << " | Yellow: " << yGem << ENDL;

            const auto& equips = m_VillageManager->GetInventory().GetEquipments();
            GPC_INFO << "Equipements: " << equips.size() << ENDL;
            for (const auto& [id, eq] : equips)
            {
                if (eq)
                {
                    GPC_INFO << "ID: " << id << " | Type: ";
                    if (dynamic_cast<ShortSword*>(eq)) GPC_INFO << "ShortSword";
                    else if (dynamic_cast<LongSword*>(eq)) GPC_INFO << "LongSword";
                    else if (dynamic_cast<Tomahawk*>(eq)) GPC_INFO << "Tomahawk";
                    else if (dynamic_cast<Bow*>(eq)) GPC_INFO << "Bow";
                    else if (dynamic_cast<Helmet*>(eq)) GPC_INFO << "Helmet";
                    else if (dynamic_cast<ChestPlate*>(eq)) GPC_INFO << "ChestPlate";
                    else if (dynamic_cast<Gloves*>(eq)) GPC_INFO << "Gloves";
                    else if (dynamic_cast<Legs*>(eq)) GPC_INFO << "Legs";
                    else if (dynamic_cast<Boots*>(eq)) GPC_INFO << "Boots";
                    else GPC_INFO << "Unknown";
                    GPC_INFO << ENDL;
                }
            }
            GPC_INFO << "========================" << ENDL;
        }

        if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::KEY_3))
        {
            m_VillageManager->GetInventory().AddResource<Wood<ResourceTier::T1>>(50);
            m_VillageManager->GetInventory().AddResource<Food>(15);
            GPC_INFO << "Ressources ajoutees" << ENDL;
        }

        if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::KEY_2))
        {
            m_VillageManager->SaveVillageData();
            GPC_INFO << "Game Save" << ENDL;
        }

        if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::KEY_1))
        {
            m_VillageManager->LoadVillageData();
            SyncVillageMap();
            GPC_INFO << "Game Load" << ENDL;
        }

        if (GPC::Inputs::IsKeyPress(GPC::Inputs::KeyCode::KEY_0))
        {
            m_VillageManager->Reset();
            SyncVillageMap();
            GPC_INFO << "Game Reset" << ENDL;
        }
    }

    void SceneVillageV1::SetupVisuals()
    {

        auto light = CreateEntityAs3D();
        AddComponent<LightingComponent>(light)->pLightData =
            LightFactory::CreateAsDirectional().Intensity(20).Diffuse(Colors::WHITE).Build();

        auto transform = GetComponent<Transform3D>(light);
        transform->LocalTransform.SetRotationYawPitchRoll( glm::radians(45.0f), glm::radians(45.0f), 0 );

    }

    void SceneVillageV1::SetupAudio() {
        auto nextMusic = CreateEntityAs3D();
        mp_VillageIntro = AddComponent<AudioSource>(nextMusic);

        mp_VillageIntro->Path = "Resources/Audio/Music/test.wav";
        mp_VillageIntro->Resource = GPC_AUDIO("ROH_village_intro");
        mp_VillageIntro->TypeVoice = AudioType::Music;
        mp_VillageIntro->Volume = SettingsGame::Get().GetVolume(AudioType::Music) / 3.0f;
        mp_VillageIntro->Looping = false;
        mp_VillageIntro->Play();

        nextMusic = CreateEntityAs3D();
        mp_VillageMusic = AddComponent<AudioSource>(nextMusic);
        mp_VillageMusic->Path = "Resources/Audio/Music/ROHvillageloop.wav";
        mp_VillageMusic->Resource = GPC_AUDIO("ROH_village");
        mp_VillageMusic->TypeVoice = AudioType::Music;
        mp_VillageMusic->Volume = SettingsGame::Get().GetVolume(AudioType::Music) / 3.0f;
        mp_VillageMusic->Looping = true;

        nextMusic = CreateEntityAs3D();
        mp_VillageNature = AddComponent<AudioSource>(nextMusic);
        mp_VillageNature->Path = "Resources/Audio/Music/nature.wav";
        mp_VillageNature->Resource = GPC_AUDIO("Nature");
        mp_VillageNature->TypeVoice = AudioType::SFX;
        mp_VillageNature->Volume = SettingsGame::Get().GetVolume(AudioType::Music) / 3.0f;
        mp_VillageNature->Looping = false;

        nextMusic = CreateEntityAs3D();
        auto audioSource = AddComponent<AudioSource>(nextMusic);
        audioSource->Resource = GPC_AUDIO("Poser_batiment");
        audioSource->Path = "Resources/Audio/SFX/Poser batiment.wav";
        audioSource->TypeVoice = AudioType::SFX;
        audioSource->Volume = SettingsGame::Get().GetVolume(AudioType::SFX) / 3.0f;
        audioSource->Looping = false;
        m_VillageManager->GetAudioSources().push_back(audioSource);

        nextMusic = CreateEntityAs3D();
        audioSource = AddComponent<AudioSource>(nextMusic);
        audioSource->Path = "Resources/Audio/SFX/Clic.wav";
        audioSource->Resource = GPC_AUDIO("Clic");
        audioSource->TypeVoice = AudioType::SFX;
        audioSource->Volume = SettingsGame::Get().GetVolume(AudioType::SFX) / 3.0f;
        audioSource->Looping = false;
        m_VillageManager->GetAudioSources().push_back(audioSource);

        nextMusic = CreateEntityAs3D();
        audioSource = AddComponent<AudioSource>(nextMusic);
        auto audioTransform = AddComponent<Transform3D>(nextMusic);
        audioTransform->LocalTransform.SetPosition({80, 5, 35});
        audioSource->Path = "Resources/Audio/Music/naturewind.wav";
        audioSource->Resource = GPC_AUDIO("Nature_only_wind");
        audioSource->TypeVoice = AudioType::SFX;
        audioSource->Volume = SettingsGame::Get().GetVolume(AudioType::SFX) / 3.0f;
        audioSource->Looping = false;
        audioSource->Is3D = true;

        nextMusic = CreateEntityAs3D();
        audioSource = AddComponent<AudioSource>(nextMusic);
        audioTransform = AddComponent<Transform3D>(nextMusic);
        audioTransform->LocalTransform.SetPosition({35, 5, 80});
        audioSource->Path = "Resources/Audio/Music/naturewind.wav";
        audioSource->Resource = GPC_AUDIO("Nature_only_wind");
        audioSource->TypeVoice = AudioType::SFX;
        audioSource->Volume = SettingsGame::Get().GetVolume(AudioType::SFX) / 3.0f;
        audioSource->Looping = false;
        audioSource->Is3D = true;

    }

    void SceneVillageV1::SetupDialogues()
    {
        auto dialogue = CreateEntityAs2D();
        mp_DialogueBehaviour = AddBehavior<DialogueBehaviour>(dialogue);
    }

    void SceneVillageV1::LoadTutoElements()
    {
        GPC_ASSETS->AddTexture("Nerd_Silene_Right",      "Resources/UI/VILLAGE/461x352/point droit.png");
        GPC_ASSETS->AddTexture("Nerd_Silene_Left",      "Resources/UI/VILLAGE/461x352/point gauche.png");

        GPC_ASSETS->AddTextureArray(
           "NerdSilene",
           461, 352,
           {
               GPC_TEXTURE("Nerd_Silene_Right"),
               GPC_TEXTURE("Nerd_Silene_Left")
           });
    }

    void SceneVillageV1::LoadUIElements()
    {

        GPC_ASSETS->AddTexture("Productivity_Crystal",      "Resources/UI/VILLAGE/218x169/ui_productivity/ui_productivity_crystal.png");
        GPC_ASSETS->AddTexture("Productivity_CrystalT1",    "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t1.png");
        GPC_ASSETS->AddTexture("Productivity_CrystalT2",    "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t2.png");
        GPC_ASSETS->AddTexture("Productivity_CrystalT3",    "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t3.png");
        GPC_ASSETS->AddTexture("Productivity_CrystalT4",    "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t4.png");
        GPC_ASSETS->AddTexture("Productivity_CrystalT5",    "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t5.png");
        GPC_ASSETS->AddTexture("Productivity_CrystalT6",    "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t6.png");

        GPC_ASSETS->AddTexture("Productivity_Fiber",   "Resources/UI/VILLAGE/218x169/ui_productivity/ui_productivity_fiber.png");
        GPC_ASSETS->AddTexture("Productivity_Food",    "Resources/UI/VILLAGE/218x169/ui_productivity/ui_productivity_food.png");
        GPC_ASSETS->AddTexture("Productivity_Iron",    "Resources/UI/VILLAGE/218x169/ui_productivity/ui_productivity_ore.png");
        GPC_ASSETS->AddTexture("Productivity_Stone",   "Resources/UI/VILLAGE/218x169/ui_productivity/ui_productivity_stone.png");
        GPC_ASSETS->AddTexture("Productivity_Wood",    "Resources/UI/VILLAGE/218x169/ui_productivity/ui_productivity_wood.png");

        GPC_ASSETS->AddTexture("Button_Buildings",     "Resources/UI/VILLAGE/218x169/ui_button_buildings.png");
        GPC_ASSETS->AddTexture("Button_Delete",        "Resources/UI/VILLAGE/218x169/ui_button_delete.png");
        GPC_ASSETS->AddTexture("Button_Expedition",    "Resources/UI/VILLAGE/218x169/ui_button_expedition.png");
        GPC_ASSETS->AddTexture("Button_Save",          "Resources/UI/VILLAGE/218x169/ui_button_save.png");
        GPC_ASSETS->AddTexture("Button_Settings",      "Resources/UI/VILLAGE/218x169/ui_button_settings.png");
        GPC_ASSETS->AddTexture("Button_Tooltip",       "Resources/UI/VILLAGE/218x169/ui_button_tooltip.png");

        GPC_ASSETS->AddTexture("Icon_Comming_Soon",         "Resources/UI/VILLAGE/218x169/ui_building_icones/_0000_comming-soon.png");
        GPC_ASSETS->AddTexture("Icon_Habitat",         "Resources/UI/VILLAGE/218x169/ui_building_icones/_0001_habitat.png");
        GPC_ASSETS->AddTexture("Icon_Forge",           "Resources/UI/VILLAGE/218x169/ui_building_icones/_0002_forge.png");
        GPC_ASSETS->AddTexture("Icon_Ferme",           "Resources/UI/VILLAGE/218x169/ui_building_icones/_0003_ferme.png");

        GPC_ASSETS->AddTexture("Forge_Cost_Crystal_T1", "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t1.png");
        GPC_ASSETS->AddTexture("Forge_Cost_Crystal_T2", "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t2.png");
        GPC_ASSETS->AddTexture("Forge_Cost_Crystal_T3", "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t3.png");
        GPC_ASSETS->AddTexture("Forge_Cost_Crystal_T4", "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t4.png");
        GPC_ASSETS->AddTexture("Forge_Cost_Crystal_T5", "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t5.png");
        GPC_ASSETS->AddTexture("Forge_Cost_Crystal_T6", "Resources/UI/VILLAGE/218x169/ui_forge/ui_forge_cost_crystal_t6.png");
        GPC_ASSETS->AddTexture("NoaLeGOOOAT",           "Resources/UI/VILLAGE/218x169/noa.png");

        GPC_ASSETS->AddTextureArray(
            "218x169",
            218, 169,
            {
                GPC_TEXTURE("Productivity_Crystal"),
                GPC_TEXTURE("Productivity_CrystalT1"),
                GPC_TEXTURE("Productivity_CrystalT2"),
                GPC_TEXTURE("Productivity_CrystalT3"),
                GPC_TEXTURE("Productivity_CrystalT4"),
                GPC_TEXTURE("Productivity_CrystalT5"),
                GPC_TEXTURE("Productivity_CrystalT6"),
                GPC_TEXTURE("Productivity_Fiber"),
                GPC_TEXTURE("Productivity_Food"),
                GPC_TEXTURE("Productivity_Iron"),
                GPC_TEXTURE("Productivity_Stone"),
                GPC_TEXTURE("Productivity_Wood"),
                GPC_TEXTURE("Button_Buildings"),
                GPC_TEXTURE("Button_Delete"),
                GPC_TEXTURE("Button_Expedition"),
                GPC_TEXTURE("Button_Save"),
                GPC_TEXTURE("Button_Settings"),
                GPC_TEXTURE("Button_Tooltip"),
                GPC_TEXTURE("Icon_Comming_Soon"),
                GPC_TEXTURE("Icon_Habitat"),
                GPC_TEXTURE("Icon_Forge"),
                GPC_TEXTURE("Icon_Ferme"),
                GPC_TEXTURE("Forge_Cost_Crystal_T1"),
                GPC_TEXTURE("Forge_Cost_Crystal_T2"),
                GPC_TEXTURE("Forge_Cost_Crystal_T3"),
                GPC_TEXTURE("Forge_Cost_Crystal_T4"),
                GPC_TEXTURE("Forge_Cost_Crystal_T5"),
                GPC_TEXTURE("Forge_Cost_Crystal_T6"),
                GPC_TEXTURE("NoaLeGOOOAT"),
            }
        );

        GPC_ASSETS->AddTexture("Resources_Background", "Resources/UI/VILLAGE/529x106/ui_resources.png");

        GPC_ASSETS->AddTextureArray(
            "529x106",
            529, 106,
            {
                GPC_TEXTURE("Resources_Background")
            }
        );

        GPC_ASSETS->AddTexture("Button_Upgrade", "Resources/UI/VILLAGE/580x157/ui_button_upgrade.png");
        GPC_ASSETS->AddTexture("Button_Upgrade_Farm", "Resources/UI/VILLAGE/580x157/ui_button_farm_upgrade.png");
        GPC_ASSETS->AddTexture("Forge_Hache",     "Resources/UI/VILLAGE/580x157/forge_button/ui_forge_hache.png");
        GPC_ASSETS->AddTexture("Forge_Button_1",  "Resources/UI/VILLAGE/580x157/forge_button/ui_forge_button_1.png");
        GPC_ASSETS->AddTexture("Forge_Button_2",  "Resources/UI/VILLAGE/580x157/forge_button/ui_forge_button_2.png");
        GPC_ASSETS->AddTexture("Forge_Button_3",  "Resources/UI/VILLAGE/580x157/forge_button/ui_forge_button_3.png");
        GPC_ASSETS->AddTexture("Forge_Button_4",  "Resources/UI/VILLAGE/580x157/forge_button/ui_forge_button_4.png");
        GPC_ASSETS->AddTexture("Forge_Button_5",  "Resources/UI/VILLAGE/580x157/forge_button/ui_forge_button_5.png");
        GPC_ASSETS->AddTexture("Forge_Button_6",  "Resources/UI/VILLAGE/580x157/forge_button/ui_forge_button_6.png");
        GPC_ASSETS->AddTexture("Forge_Button_7",  "Resources/UI/VILLAGE/580x157/forge_button/ui_forge_button_7.png");
        GPC_ASSETS->AddTexture("Forge_Gauntlet",  "Resources/UI/VILLAGE/580x157/forge_button/ui_forge_gauntlet.png");

        GPC_ASSETS->AddTextureArray(
            "580x157",
            580, 157,
            {
                GPC_TEXTURE("Button_Upgrade"),
                GPC_TEXTURE("Button_Upgrade_Farm"),
                GPC_TEXTURE("Forge_Hache"),
                GPC_TEXTURE("Forge_Button_1"),
                GPC_TEXTURE("Forge_Button_2"),
                GPC_TEXTURE("Forge_Button_3"),
                GPC_TEXTURE("Forge_Button_4"),
                GPC_TEXTURE("Forge_Button_5"),
                GPC_TEXTURE("Forge_Button_6"),
                GPC_TEXTURE("Forge_Button_7"),
                GPC_TEXTURE("Forge_Gauntlet"),
            }
        );

        GPC_ASSETS->AddTexture("Background_Tooltip",                   "Resources/UI/VILLAGE/786x860/ui_tooltip.png");
        GPC_ASSETS->AddTexture("Background_Upgrade",                   "Resources/UI/VILLAGE/786x860/ui_upgrade.png");
        GPC_ASSETS->AddTexture("Background_Buildings",                 "Resources/UI/VILLAGE/786x860/ui_buildings.png");
        GPC_ASSETS->AddTexture("Background_Farm_Text",                 "Resources/UI/VILLAGE/786x860/ui_farm_text.png");
        GPC_ASSETS->AddTexture("Background_Forge_Panel",               "Resources/UI/VILLAGE/786x860/ui_forge_panel.png");
        GPC_ASSETS->AddTexture("Background_Forge_Panel_Upgrade",       "Resources/UI/VILLAGE/786x860/ui_forge_upgrade_panel.png");

        GPC_ASSETS->AddTextureArray(
            "786x860",
            786, 860,
            {
                GPC_TEXTURE("Background_Tooltip"),
                GPC_TEXTURE("Background_Upgrade"),
                GPC_TEXTURE("Background_Buildings"),
                GPC_TEXTURE("Background_Farm_Text"),
                GPC_TEXTURE("Background_Forge_Panel"),
                GPC_TEXTURE("Background_Forge_Panel_Upgrade"),
            }
        );

        GPC_ASSETS->AddTexture("EditMode_Building",     "Resources/UI/VILLAGE/250x180/ui_edit_button_buildings.png");
        GPC_ASSETS->AddTexture("EditMode_Cancel",       "Resources/UI/VILLAGE/250x180/ui_edit_cancel.png");
        GPC_ASSETS->AddTexture("EditMode_Clear",        "Resources/UI/VILLAGE/250x180/ui_edit_clear.png");
        GPC_ASSETS->AddTexture("EditMode_Number",       "Resources/UI/VILLAGE/250x180/ui_edit_number.png");
        GPC_ASSETS->AddTexture("EditMode_Save",         "Resources/UI/VILLAGE/250x180/ui_edit_save.png");
        GPC_ASSETS->AddTexture("EditMode_Icon",         "Resources/UI/VILLAGE/250x180/ui_editmode_icon.png");

        GPC_ASSETS->AddTextureArray(
            "250x180",
            250, 180,
            {
                GPC_TEXTURE("EditMode_Building" ),
                GPC_TEXTURE("EditMode_Cancel"   ),
                GPC_TEXTURE("EditMode_Clear"    ),
                GPC_TEXTURE("EditMode_Number"   ),
                GPC_TEXTURE("EditMode_Save"     ),
                GPC_TEXTURE("EditMode_Icon"     )
            }
        );

        GPC_ASSETS->AddTexture("EditMode_Panel",         "Resources/UI/VILLAGE/1551x277/ui_edit_panel.png");

        GPC_ASSETS->AddTextureArray(
            "1551x277",
            1551, 277,
            {
                GPC_TEXTURE("EditMode_Panel" ),
            }
        );
    }

    void SceneVillageV1::LoadAnimations()
    {
        // BUILDING BUTTONS

        GPC_ASSETS->AddTexture("ANIM00_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00000.png");
        GPC_ASSETS->AddTexture("ANIM01_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00001.png");
        GPC_ASSETS->AddTexture("ANIM02_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00002.png");
        GPC_ASSETS->AddTexture("ANIM03_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00003.png");
        GPC_ASSETS->AddTexture("ANIM04_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00004.png");
        GPC_ASSETS->AddTexture("ANIM05_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00005.png");
        GPC_ASSETS->AddTexture("ANIM06_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00006.png");
        GPC_ASSETS->AddTexture("ANIM07_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00007.png");
        GPC_ASSETS->AddTexture("ANIM08_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00008.png");
        GPC_ASSETS->AddTexture("ANIM09_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00009.png");
        GPC_ASSETS->AddTexture("ANIM10_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00010.png");
        GPC_ASSETS->AddTexture("ANIM11_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00011.png");
        GPC_ASSETS->AddTexture("ANIM12_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00012.png");
        GPC_ASSETS->AddTexture("ANIM13_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00013.png");
        GPC_ASSETS->AddTexture("ANIM14_Building_Button","Resources/UI/VILLAGE/218x169/ANIM/ui_button_buildings/ui_button_buildings_00014.png");
        GPC_ASSETS->AddTextureArray(
                    "ANIMATIONS_Building_Button",
                    218, 169,
                    {
                        GPC_TEXTURE("ANIM00_Building_Button"),
                        GPC_TEXTURE("ANIM01_Building_Button"),
                        GPC_TEXTURE("ANIM02_Building_Button"),
                        GPC_TEXTURE("ANIM03_Building_Button"),
                        GPC_TEXTURE("ANIM04_Building_Button"),
                        GPC_TEXTURE("ANIM05_Building_Button"),
                        GPC_TEXTURE("ANIM06_Building_Button"),
                        GPC_TEXTURE("ANIM07_Building_Button"),
                        GPC_TEXTURE("ANIM08_Building_Button"),
                        GPC_TEXTURE("ANIM09_Building_Button"),
                        GPC_TEXTURE("ANIM10_Building_Button"),
                        GPC_TEXTURE("ANIM11_Building_Button"),
                        GPC_TEXTURE("ANIM12_Building_Button"),
                        GPC_TEXTURE("ANIM13_Building_Button"),
                        GPC_TEXTURE("ANIM14_Building_Button"),
                    }
                );

        // RESSOURCES BACKGROUND

        GPC_ASSETS->AddTexture("ANIM00_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00000.png");
        GPC_ASSETS->AddTexture("ANIM01_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00001.png");
        GPC_ASSETS->AddTexture("ANIM02_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00002.png");
        GPC_ASSETS->AddTexture("ANIM03_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00003.png");
        GPC_ASSETS->AddTexture("ANIM04_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00004.png");
        GPC_ASSETS->AddTexture("ANIM05_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00005.png");
        GPC_ASSETS->AddTexture("ANIM06_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00006.png");
        GPC_ASSETS->AddTexture("ANIM07_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00007.png");
        GPC_ASSETS->AddTexture("ANIM08_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00008.png");
        GPC_ASSETS->AddTexture("ANIM09_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00009.png");
        GPC_ASSETS->AddTexture("ANIM10_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00010.png");
        GPC_ASSETS->AddTexture("ANIM11_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00011.png");
        GPC_ASSETS->AddTexture("ANIM12_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00012.png");
        GPC_ASSETS->AddTexture("ANIM13_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00013.png");
        GPC_ASSETS->AddTexture("ANIM14_Resources_Background", "Resources/UI/VILLAGE/529x106/ANIM/ui_ressources/ui_ressources_00014.png");
        GPC_ASSETS->AddTextureArray(
                    "ANIMATION_Resources_Background",
                    529, 106,
                    {
                        GPC_TEXTURE("ANIM00_Resources_Background"),
                        GPC_TEXTURE("ANIM01_Resources_Background"),
                        GPC_TEXTURE("ANIM02_Resources_Background"),
                        GPC_TEXTURE("ANIM03_Resources_Background"),
                        GPC_TEXTURE("ANIM04_Resources_Background"),
                        GPC_TEXTURE("ANIM05_Resources_Background"),
                        GPC_TEXTURE("ANIM06_Resources_Background"),
                        GPC_TEXTURE("ANIM07_Resources_Background"),
                        GPC_TEXTURE("ANIM08_Resources_Background"),
                        GPC_TEXTURE("ANIM09_Resources_Background"),
                        GPC_TEXTURE("ANIM10_Resources_Background"),
                        GPC_TEXTURE("ANIM11_Resources_Background"),
                        GPC_TEXTURE("ANIM12_Resources_Background"),
                        GPC_TEXTURE("ANIM13_Resources_Background"),
                        GPC_TEXTURE("ANIM14_Resources_Background"),
                    }
                );

        // BUILDING PANEL

        GPC_ASSETS->AddTexture("ANIM00_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00000.png");
        GPC_ASSETS->AddTexture("ANIM01_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00001.png");
        GPC_ASSETS->AddTexture("ANIM02_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00002.png");
        GPC_ASSETS->AddTexture("ANIM03_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00003.png");
        GPC_ASSETS->AddTexture("ANIM04_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00004.png");
        GPC_ASSETS->AddTexture("ANIM05_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00005.png");
        GPC_ASSETS->AddTexture("ANIM06_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00006.png");
        GPC_ASSETS->AddTexture("ANIM07_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00007.png");
        GPC_ASSETS->AddTexture("ANIM08_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00008.png");
        GPC_ASSETS->AddTexture("ANIM09_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00009.png");
        GPC_ASSETS->AddTexture("ANIM10_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00010.png");
        GPC_ASSETS->AddTexture("ANIM11_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00011.png");
        GPC_ASSETS->AddTexture("ANIM12_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00012.png");
        GPC_ASSETS->AddTexture("ANIM13_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00013.png");
        GPC_ASSETS->AddTexture("ANIM14_Building", "Resources/UI/VILLAGE/786x860/ANIM/ui_buildings/ui_buildings_00014.png");

        GPC_ASSETS->AddTextureArray(
            "ANIMATION_Building",
            786, 860,
            {
                GPC_TEXTURE("ANIM00_Building"),
                GPC_TEXTURE("ANIM01_Building"),
                GPC_TEXTURE("ANIM02_Building"),
                GPC_TEXTURE("ANIM03_Building"),
                GPC_TEXTURE("ANIM04_Building"),
                GPC_TEXTURE("ANIM05_Building"),
                GPC_TEXTURE("ANIM06_Building"),
                GPC_TEXTURE("ANIM07_Building"),
                GPC_TEXTURE("ANIM08_Building"),
                GPC_TEXTURE("ANIM09_Building"),
                GPC_TEXTURE("ANIM10_Building"),
                GPC_TEXTURE("ANIM11_Building"),
                GPC_TEXTURE("ANIM12_Building"),
                GPC_TEXTURE("ANIM13_Building"),
                GPC_TEXTURE("ANIM14_Building"),
            }
        );

        // TOOLTIPS PANEL

        GPC_ASSETS->AddTexture("ANIM00_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00000.png");
        GPC_ASSETS->AddTexture("ANIM01_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00001.png");
        GPC_ASSETS->AddTexture("ANIM02_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00002.png");
        GPC_ASSETS->AddTexture("ANIM03_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00003.png");
        GPC_ASSETS->AddTexture("ANIM04_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00004.png");
        GPC_ASSETS->AddTexture("ANIM05_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00005.png");
        GPC_ASSETS->AddTexture("ANIM06_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00006.png");
        GPC_ASSETS->AddTexture("ANIM07_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00007.png");
        GPC_ASSETS->AddTexture("ANIM08_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00008.png");
        GPC_ASSETS->AddTexture("ANIM09_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00009.png");
        GPC_ASSETS->AddTexture("ANIM10_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00010.png");
        GPC_ASSETS->AddTexture("ANIM11_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00011.png");
        GPC_ASSETS->AddTexture("ANIM12_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00012.png");
        GPC_ASSETS->AddTexture("ANIM13_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00013.png");
        GPC_ASSETS->AddTexture("ANIM14_ToolTips", "Resources/UI/VILLAGE/786x860/ANIM/ui_tooltip/ui_tooltip_00014.png");

        GPC_ASSETS->AddTextureArray(
            "ANIMATION_ToolTips",
            786, 860,
            {
                GPC_TEXTURE("ANIM00_ToolTips"),
                GPC_TEXTURE("ANIM01_ToolTips"),
                GPC_TEXTURE("ANIM02_ToolTips"),
                GPC_TEXTURE("ANIM03_ToolTips"),
                GPC_TEXTURE("ANIM04_ToolTips"),
                GPC_TEXTURE("ANIM05_ToolTips"),
                GPC_TEXTURE("ANIM06_ToolTips"),
                GPC_TEXTURE("ANIM07_ToolTips"),
                GPC_TEXTURE("ANIM08_ToolTips"),
                GPC_TEXTURE("ANIM09_ToolTips"),
                GPC_TEXTURE("ANIM10_ToolTips"),
                GPC_TEXTURE("ANIM11_ToolTips"),
                GPC_TEXTURE("ANIM12_ToolTips"),
                GPC_TEXTURE("ANIM13_ToolTips"),
                GPC_TEXTURE("ANIM14_ToolTips"),
            }
        );
    }

    ResourceCountsSnapshot SceneVillageV1::CaptureResourceCounts()
    {
        ResourceCountsSnapshot snap{};
        if (!m_VillageManager) return snap;
        Inventory& inv = m_VillageManager->GetInventory();

        auto sumFiber = inv.GetResourceCount<Fiber<ResourceTier::T1>>()
                      + inv.GetResourceCount<Fiber<ResourceTier::T2>>()
                      + inv.GetResourceCount<Fiber<ResourceTier::T3>>()
                      + inv.GetResourceCount<Fiber<ResourceTier::T4>>()
                      + inv.GetResourceCount<Fiber<ResourceTier::T5>>();

        auto sumWood = inv.GetResourceCount<Wood<ResourceTier::T1>>()
                     + inv.GetResourceCount<Wood<ResourceTier::T2>>()
                     + inv.GetResourceCount<Wood<ResourceTier::T3>>()
                     + inv.GetResourceCount<Wood<ResourceTier::T4>>()
                     + inv.GetResourceCount<Wood<ResourceTier::T5>>();

        auto sumStone = inv.GetResourceCount<Stone<ResourceTier::T1>>()
                      + inv.GetResourceCount<Stone<ResourceTier::T2>>()
                      + inv.GetResourceCount<Stone<ResourceTier::T3>>()
                      + inv.GetResourceCount<Stone<ResourceTier::T4>>()
                      + inv.GetResourceCount<Stone<ResourceTier::T5>>();

        auto sumOre = inv.GetResourceCount<Metal<ResourceTier::T1>>()
                    + inv.GetResourceCount<Metal<ResourceTier::T2>>()
                    + inv.GetResourceCount<Metal<ResourceTier::T3>>()
                    + inv.GetResourceCount<Metal<ResourceTier::T4>>()
                    + inv.GetResourceCount<Metal<ResourceTier::T5>>();

        snap.Fiber = sumFiber;
        snap.Wood  = sumWood;
        snap.Food  = inv.GetResourceCount<Food>();
        snap.Stone = sumStone;
        snap.Ore   = sumOre;
        return snap;
    }

    void SceneVillageV1::FindLifeTreeBehavior()
    {
        if (!m_VillageManager) return;
        for (auto& data : m_VillageManager->GetBuildingsData())
        {
            if (data.Type == BuildingType::LifeTree && data.LinkedBehavior)
            {
                mp_LifeTreeBehavior = data.LinkedBehavior;
                return;
            }
        }
    }

    void SceneVillageV1::SetHierarchyEnabled(EntityID root, bool enabled)
    {
        if (!HasComponent<Transform2D>(root)) return;

        if (HasComponent<UiCanvas>(root)) GetComponent<UiCanvas>(root)->IsEnable = enabled;
        if (HasComponent<UiSprite>(root)) GetComponent<UiSprite>(root)->IsEnable = enabled;
        if (HasComponent<UiText>(root))   GetComponent<UiText>(root)->IsEnable   = enabled;

        Transform2D* pTransform = GetComponent<Transform2D>(root);
        if (pTransform == nullptr) return;

        for (uint32_t i = 0; i < pTransform->GetChildCount(); ++i)
        {
            Transform2D* pChild = pTransform->GetChild(i);
            if (pChild == nullptr) continue;
            SetHierarchyEnabled(pChild->GetEntityID(), enabled);
        }
    }

    void SceneVillageV1::SetupLaunchingScreenUI()
    {
        UiBuilder b(this);

        mp_LaunchingScreen = b
            .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
            .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
            .Tint(Colors::TRANSPARENT)
            .FitToScreen(true)
            .BuildCanvas();
        if (!mp_LaunchingScreen) return;

        mp_LaunchingText = b.ChildOf(mp_LaunchingScreen->GetEntityID())
            .At(Anchors::TOP_MIDDLE, { 0.0f, 0.0f })
            .Font(UiTheme::DefaultFont, 30)
            .Tint(Color(255, 255, 255, 255))
            .Text("MISE EN FONCTION DU VILLAGE")
            .BuildText();

        mp_LaunchingBackground = b.ChildOf(mp_LaunchingScreen->GetEntityID())
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
            .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
            .Scale({2.0f, 2.0f})
            .Tint(Color(0, 0, 0, 0))
            .BuildSprite();


        if (mp_LaunchingText)
        {
            mp_LaunchingText->CalcElementSize();
            const float textHeight = mp_LaunchingText->GetSize().y;
            Transform2D* tr = GetComponent<Transform2D>(mp_LaunchingText->GetEntityID());
            if (tr)
            {
                glm::vec3 p = tr->LocalTransform.GetPosition();
                p.y = textHeight;
                tr->LocalTransform.SetPosition(p);
            }
        }

        SetHierarchyEnabled(mp_LaunchingScreen->GetEntityID(), false);
    }

    void SceneVillageV1::SetupRecapPanelUI()
    {
        UiBuilder b(this);

        mp_RecapPanel = b
            .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
            .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
            .Tint(Colors::TRANSPARENT)
            .FitToScreen(true)
            .BuildCanvas();
        if (!mp_RecapPanel) return;
        const EntityID rootId = mp_RecapPanel->GetEntityID();

        UiSprite* bgSprite = b.ChildOf(rootId)
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
            .Scale({ 0.55f, 0.55f })
            .Tint(Color(255, 255, 255, 128))
            .Sprite("1443x998", "Settings_Background")
            .BuildSprite();

        const EntityID panelId = bgSprite ? bgSprite->GetEntityID() : rootId;

        b.ChildOf(panelId)
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, -180.0f })
            .Font(UiTheme::DefaultFont, 36)
            .Tint(UiTheme::DarkText)
            .Text("PRODUCTION DU VILLAGE")
            .BuildText();

        b.ChildOf(panelId)
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, -130.0f })
            .Font(UiTheme::DefaultFont, 22)
            .Tint(UiTheme::DarkText)
            .Text("Pendant votre absence, le village a genere :")
            .BuildText();

        struct LineDef { const char* Label; UiText** ppText; float OffsetY; };
        LineDef lines[] = {
            { "Bois        : ", &mp_RecapWoodText,  -60.0f },
            { "Nourriture  : ", &mp_RecapFoodText,  -20.0f },
            { "Pierre      : ", &mp_RecapStoneText,  20.0f },
            { "Fibre       : ", &mp_RecapFiberText,  60.0f },
            { "Minerai     : ", &mp_RecapOreText,   100.0f },
        };

        for (auto& def : lines)
        {
            b.ChildOf(panelId)
                .At(Anchors::MIDDLE_MIDDLE, { -100.0f, def.OffsetY })
                .Font(UiTheme::DefaultFont, 24)
                .Tint(UiTheme::DarkText)
                .Text(def.Label)
                .BuildText();

            *def.ppText = b.ChildOf(panelId)
                .At(Anchors::MIDDLE_MIDDLE, { 100.0f, def.OffsetY })
                .Font(UiTheme::DefaultFont, 24)
                .Tint(UiTheme::DarkText)
                .Text("0")
                .BuildText();
        }

        UiSprite* validBtnSprite = b.ChildOf(panelId)
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 180.0f })
            .Size({ 220.0f, 60.0f })
            .Tint(Color(60, 90, 60, 255))
            .BuildSprite();

        if (validBtnSprite)
        {
            b.ChildOf(validBtnSprite->GetEntityID())
                .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
                .Font(UiTheme::DefaultFont, 28)
                .Tint(Colors::WHITE)
                .Text("VALIDER")
                .BuildText();

            UiButton* validBtn = b.BuildButton(validBtnSprite);
            if (validBtn)
            {
                validBtn->OnClick.ConnectLambda([this, validBtnSprite]()
                {
                    UiAnim::ScaleTo(*this, validBtnSprite, { 1.0f, 1.0f }, 0.0f);
                    CloseRecapPanel();
                });
                validBtn->OnHover.ConnectLambda([this, validBtnSprite]()
                {
                    UiAnim::ScaleTo(*this, validBtnSprite, { 1.05f, 1.05f }, 0.15f);
                });
                validBtn->OnLeave.ConnectLambda([this, validBtnSprite]()
                {
                    UiAnim::ScaleTo(*this, validBtnSprite, { 1.0f, 1.0f }, 0.15f);
                });
            }
        }

        SetHierarchyEnabled(rootId, false);
    }

    void SceneVillageV1::FocusCameraOnLifeTree()
    {
        if (!mp_LifeTreeBehavior) FindLifeTreeBehavior();
        if (!mp_LifeTreeBehavior) return;

        mp_LifeTreeBehavior->Focus({ 0.0f, 0.0f });
    }

    void SceneVillageV1::SetupReturnTransition()
    {
        EntityID e = CreateEntityAs3D();
        mp_ReturnTransition = AddBehavior<TransiFeuillle>(e);
    }

    void SceneVillageV1::PlayReturnRevealSequence()
    {
        FocusCameraOnLifeTree();

        if (mp_ReturnTransition == nullptr)
        {
            m_RecapPending = true;
            StartTreePulse();
            return;
        }

        if (mp_LaunchingScreen == nullptr) SetupLaunchingScreenUI();
        if (mp_LaunchingScreen == nullptr)
        {
            m_RecapPending = true;
            StartTreePulse();
            return;
        }

        m_LaunchInProgress = true;
        mp_ReturnTransition->SnapToClosed();

        SceneVillageV1* self = this;
        const EntityID anchorEid = mp_LaunchingScreen->GetEntityID();

        UiAnimSlot revealSlot{};
        revealSlot.Delay    = 0.5f;
        revealSlot.Duration = 1.75f;
        revealSlot.OnStart = [self]()
        {
            if (self->mp_ReturnTransition == nullptr) return;
            const float now = static_cast<float>(self->m_Clock.GetTime());
            self->mp_ReturnTransition->LaunchOpenAnimation(now, self, nullptr);
        };
        revealSlot.OnComplete = [self]()
        {
            self->m_LaunchInProgress = false;
            self->m_RecapPending     = true;
            self->StartTreePulse();
        };
        UiAnim::PushAnim(*this, anchorEid, std::move(revealSlot));
    }

    void SceneVillageV1::PlayLaunchExpeditionSequence(SceneExpeditionStartCtx* pLoadout)
    {
        if (pLoadout == nullptr) return;

        m_VillageController->StartInspection();
        m_LaunchInProgress = true;

        if (m_UiController)
        {
            m_UiController->SetState(VillageUiState::Idle);
        }

        if (m_VillageManager)
        {
            m_VillageManager->SetAllBuildingsActive(true);
            m_VillageManager->SaveVillageData();
        }

        if (!mp_LaunchingScreen) SetupLaunchingScreenUI();
        if (!mp_LaunchingScreen)
        {
            SceneChangeCtx ctx{};
            ctx.pData    = pLoadout;
            ctx.DataSize = sizeof(SceneExpeditionStartCtx);
            SwapToScene("Expedition", ctx);
            return;
        }

        SetHierarchyEnabled(mp_LaunchingScreen->GetEntityID(), true);
        if (mp_LaunchingBackground) mp_LaunchingBackground->SetTint(Color(0, 0, 0, 255));
        if (mp_LaunchingText)       mp_LaunchingText->SetTint(Color(0, 0, 0, 255));

        const EntityID screenEid = mp_LaunchingScreen->GetEntityID();
        SceneExpeditionStartCtx* pCaptured = pLoadout;
        SceneVillageV1* self = this;

        const auto applyAlphaBg = [self](uint8_t a)
        {
            if (self->mp_LaunchingBackground) self->mp_LaunchingBackground->SetTint(Color(0, 0, 0, a));
        };

        constexpr float kHoldOpaque1MinDuration = 1.0f;

        UiAnimSlot holdOpaque1{};
        holdOpaque1.Delay    = 0.0f;
        holdOpaque1.Duration = kHoldOpaque1MinDuration;
        holdOpaque1.OnStart  = [self]()
        {
            self->FocusCameraOnLifeTree();
        };
        holdOpaque1.Setter   = [applyAlphaBg](float)
        {
            applyAlphaBg(255);
        };
        holdOpaque1.OnComplete = [self, pCaptured]()
        {
            self->mp_PendingLaunchLoadout = pCaptured;
            self->m_LaunchAwaitingCamFocus = true;
        };
        UiAnim::PushAnim(*this, screenEid, std::move(holdOpaque1));
    }

    void SceneVillageV1::PushLaunchPostFocusSlots()
    {
        if (!mp_LaunchingScreen) return;
        SceneExpeditionStartCtx* pCaptured = mp_PendingLaunchLoadout;
        if (!pCaptured) return;

        const EntityID screenEid = mp_LaunchingScreen->GetEntityID();
        Scene* pScene = this;
        SceneVillageV1* self = this;

        const auto applyBgAndText = [self](uint8_t bgAlpha, uint8_t textRgb)
        {
            if (self->mp_LaunchingBackground) self->mp_LaunchingBackground->SetTint(Color(0, 0, 0, bgAlpha));
            if (self->mp_LaunchingText)       self->mp_LaunchingText->SetTint(Color(textRgb, textRgb, textRgb, 255));
        };

        constexpr float kFadeOutDuration         = 2.0f;
        constexpr float kHoldTransparentDuration = 0.5f;
        constexpr float kFadeInDuration          = 2.0f;
        constexpr float kHoldOpaque2Duration     = 0.05f;

        constexpr float kZoomStart = 10.0f;
        constexpr float kZoomEnd   = 15.0f;
        const float kDezoomDuration = kFadeOutDuration + kHoldTransparentDuration + kFadeInDuration;

        if (m_VillageController) m_VillageController->StartTransition();

        if (m_VillageManager)
        {
            m_VillageManager->StaggerSwapToActiveTextures(kFadeOutDuration);
        }

        float cursor = 0.0f;

        UiAnimSlot dezoom{};
        dezoom.Delay    = cursor;
        dezoom.Duration = kDezoomDuration;
        dezoom.Ease     = Tweening::EasingType::Linear;
        dezoom.Setter = [self](float t)
        {
            if (self->mp_MainCameraBehaviour)
            {
                self->mp_MainCameraBehaviour->Zoom = kZoomStart + (kZoomEnd - kZoomStart) * t;
            }
        };
        UiAnim::PushAnim(*this, screenEid, std::move(dezoom));

        UiAnimSlot fadeOutBg{};
        fadeOutBg.Delay    = cursor;
        fadeOutBg.Duration = kFadeOutDuration;
        fadeOutBg.Ease     = Tweening::EasingType::EaseInQuad;
        fadeOutBg.Setter = [self](float t)
        {
            if (self->mp_LaunchingBackground)
            {
                const uint8_t bgA = static_cast<uint8_t>(255.0f * (1.0f - t));
                self->mp_LaunchingBackground->SetTint(Color(0, 0, 0, bgA));
            }
        };
        UiAnim::PushAnim(*this, screenEid, std::move(fadeOutBg));

        UiAnimSlot textColorize{};
        textColorize.Delay    = cursor;
        textColorize.Duration = kFadeOutDuration;
        textColorize.Ease     = Tweening::EasingType::Linear;
        textColorize.Setter = [self](float t)
        {
            if (self->mp_LaunchingText)
            {
                const uint8_t c = static_cast<uint8_t>(255.0f * t);
                self->mp_LaunchingText->SetTint(Color(c, c, c, 255));
            }
        };
        UiAnim::PushAnim(*this, screenEid, std::move(textColorize));
        cursor += kFadeOutDuration;

        UiAnimSlot holdTransparent{};
        holdTransparent.Delay    = cursor;
        holdTransparent.Duration = kHoldTransparentDuration;
        holdTransparent.Setter   = [applyBgAndText](float)
        {
            applyBgAndText(0, 255);
        };
        UiAnim::PushAnim(*this, screenEid, std::move(holdTransparent));
        cursor += kHoldTransparentDuration;

        UiAnimSlot fadeInBg{};
        fadeInBg.Delay    = cursor;
        fadeInBg.Duration = kFadeInDuration;
        fadeInBg.Ease     = Tweening::EasingType::EaseInQuad;
        fadeInBg.Setter = [applyBgAndText](float t)
        {
            const uint8_t bgA   = static_cast<uint8_t>(255.0f * t);
            const uint8_t textC = static_cast<uint8_t>(255.0f * (1.0f - t));
            applyBgAndText(bgA, textC);
        };
        UiAnim::PushAnim(*this, screenEid, std::move(fadeInBg));
        cursor += kFadeInDuration;

        UiAnimSlot holdOpaque2{};
        holdOpaque2.Delay    = cursor;
        holdOpaque2.Duration = kHoldOpaque2Duration;
        holdOpaque2.Setter   = [applyBgAndText](float)
        {
            applyBgAndText(255, 0);
        };
        holdOpaque2.OnComplete = [this, pScene, pCaptured]()
        {
            SceneChangeCtx ctx{};
            ctx.pData    = pCaptured;
            ctx.DataSize = sizeof(SceneExpeditionStartCtx);
            m_VillageController->EndInspection();
            m_VillageController->EndTransition();
            pScene->SwapToScene("Expedition", ctx);
        };
        UiAnim::PushAnim(*this, screenEid, std::move(holdOpaque2));

        mp_PendingLaunchLoadout = nullptr;
    }

    void SceneVillageV1::OpenRecapPanel(const ResourceCountsSnapshot& delta)
    {
        if (!mp_RecapPanel) return;

        if (mp_RecapWoodText)  mp_RecapWoodText->SetText(std::to_string(delta.Wood));
        if (mp_RecapFoodText)  mp_RecapFoodText->SetText(std::to_string(delta.Food));
        if (mp_RecapStoneText) mp_RecapStoneText->SetText(std::to_string(delta.Stone));
        if (mp_RecapFiberText) mp_RecapFiberText->SetText(std::to_string(delta.Fiber));
        if (mp_RecapOreText)   mp_RecapOreText->SetText(std::to_string(delta.Ore));

        SetHierarchyEnabled(mp_RecapPanel->GetEntityID(), true);
    }

    void SceneVillageV1::CloseRecapPanel()
    {
        if (mp_RecapPanel) SetHierarchyEnabled(mp_RecapPanel->GetEntityID(), false);

        StopTreePulse();
        m_RecapPending = false;

        if (mp_LifeTreeBehavior)
        {
            mp_LifeTreeBehavior->OnRecapInteract = nullptr;
            mp_LifeTreeBehavior->CloseOpened();
        }

        if (m_VillageController)
        {
            m_VillageController->EndInspection();
        }

        if (m_VillageManager)
        {
            m_VillageManager->StaggerSwapToInactiveTextures(1.5f);
            m_VillageManager->SetAllBuildingsActive(false);
            m_VillageManager->SaveVillageData();
        }
    }

    void SceneVillageV1::StartTreePulse()
    {
        if (!mp_LifeTreeBehavior) FindLifeTreeBehavior();
        if (!mp_LifeTreeBehavior) return;
        if (!mp_LifeTreeBehavior->Render3DComponent_) return;

        EntityID spriteId = mp_LifeTreeBehavior->Render3DComponent_->GetEntityID();
        Transform3D* tr = GetComponent<Transform3D>(spriteId);
        if (!tr) return;

        if (!m_LifeTreeBaseScaleCaptured)
        {
            m_LifeTreeBaseScale         = tr->LocalTransform.GetScale();
            m_LifeTreeBasePos           = tr->LocalTransform.GetPosition();
            m_LifeTreeBaseScaleCaptured = true;
        }

        m_TreePulseActive = true;
    }

    void SceneVillageV1::StopTreePulse()
    {
        m_TreePulseActive = false;

        if (!m_LifeTreeBaseScaleCaptured) return;
        if (!mp_LifeTreeBehavior) return;
        if (!mp_LifeTreeBehavior->Render3DComponent_) return;

        EntityID spriteId = mp_LifeTreeBehavior->Render3DComponent_->GetEntityID();
        Transform3D* tr = GetComponent<Transform3D>(spriteId);
        if (tr)
        {
            tr->LocalTransform.SetScale(m_LifeTreeBaseScale);
            tr->LocalTransform.SetPosition(m_LifeTreeBasePos);
        }
    }

    void SceneVillageV1::ResetSequenceState()
    {
        m_LaunchInProgress       = false;
        m_LaunchAwaitingCamFocus = false;
        mp_PendingLaunchLoadout  = nullptr;
        m_RecapPending           = false;

        if (mp_LaunchingScreen)
        {
            UiAnim::Cancel(*this, mp_LaunchingScreen->GetEntityID());
            if (mp_LaunchingBackground) mp_LaunchingBackground->SetTint(Color(0, 0, 0, 0));
            if (mp_LaunchingText)       mp_LaunchingText->SetTint(Color(255, 255, 255, 0));
            SetHierarchyEnabled(mp_LaunchingScreen->GetEntityID(), false);
        }

        if (mp_RecapPanel)
        {
            SetHierarchyEnabled(mp_RecapPanel->GetEntityID(), false);
        }

        StopTreePulse();

        if (mp_LifeTreeBehavior)
        {
            mp_LifeTreeBehavior->OnRecapInteract = nullptr;
        }
    }
    
    void SceneVillageV1::LoadParticles()
    {
        GPC_ASSETS->AddShader("Compute Shader Particle", "particles.comp", Shader::Type::COMPUTE);

        {
            GPC_ASSETS->AddParticleProgram(
              "Luciole Particle Program",
              {
                  .Looping                = true,
                  .MaxParticle            = 128,
                  .Mode                   = ParticleEmissionMode::OVER_TIME,
                  .Shape                  = ParticleEmissionShape::SHPERE,
                  .StartMinPosition       = glm::vec3(-35.0f, 5.0f, -35.0f),
                  .StartMaxPosition       = glm::vec3(35.0f, 5.0f, 35.0f),
                  .StartAfterSeconds      = 0.0f,
                  .LifeTimeMin            = 20.0f,
                  .LifeTimeMax            = 30.0f,
                  .StartMinSpeed          = glm::vec3(-0.2f),
                  .StartMaxSpeed          = glm::vec3(0.2f),
                  .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                  .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                  .StartMinSize           = glm::vec3(0.08),
                  .StartMaxSize           = glm::vec3(0.12),
                  .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                  .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                  .StartMaxColor          = 0xFFFF00C8,
                  .StartMinColor          = 0xFFF44F7D
              },
              GPC_SHADER("Compute Shader Particle")
          );

            EntityID lucioleEmitter = CreateEntityAs3D();
            auto transform = GetComponent<Transform3D>(lucioleEmitter);
            transform->LocalTransform.AddPosition(35, 0, 35);
            auto emitter = AddComponent<ParticleEmitter>(lucioleEmitter);
            emitter->pGeometry = GPC_MESH("Sphere");
            emitter->pComputeProgram = &GPC_COMPUTE_PROGRAM("Luciole Particle Program")->Program;
            emitter->SetLooping(true);
            emitter->AutoStart();
        }

        {
            GPC_ASSETS->AddParticleProgram(
              "WorldBorder0 Particle Program",
              {
                  .Looping                = true,
                  .MaxParticle            = 128,
                  .Mode                   = ParticleEmissionMode::OVER_TIME,
                  .Shape                  = ParticleEmissionShape::CUBE,
                  .StartMinPosition       = glm::vec3(-35.0f, 5.0f, -5.0f),
                  .StartMaxPosition       = glm::vec3(35.0f, 5.0f, 5.0f),
                  .StartAfterSeconds      = 0.0f,
                  .LifeTimeMin            = 10.0f,
                  .LifeTimeMax            = 20.0f,
                  .StartMinSpeed          = glm::vec3(0.0, 0.1, 0.0),
                  .StartMaxSpeed          = glm::vec3(0.0, 0.3, 0.0),
                  .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                  .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                  .StartMinSize           = glm::vec3(0.08),
                  .StartMaxSize           = glm::vec3(0.12),
                  .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                  .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                  .StartMaxColor          = 0x228B22FF,
                  .StartMinColor          = 0x31D4928C
              },
              GPC_SHADER("Compute Shader Particle")
          );

            EntityID lucioleEmitter = CreateEntityAs3D();
            auto transform = GetComponent<Transform3D>(lucioleEmitter);
            transform->LocalTransform.AddPosition(35, 14, 70);
            auto emitter = AddComponent<ParticleEmitter>(lucioleEmitter);
            emitter->pGeometry = GPC_MESH("Sphere");
            emitter->pComputeProgram = &GPC_COMPUTE_PROGRAM("WorldBorder0 Particle Program")->Program;
            emitter->SetLooping(true);
            emitter->AutoStart();
        }

        {
            GPC_ASSETS->AddParticleProgram(
              "WorldBorder1 Particle Program",
              {
                  .Looping                = true,
                  .MaxParticle            = 128,
                  .Mode                   = ParticleEmissionMode::OVER_TIME,
                  .Shape                  = ParticleEmissionShape::CUBE,
                  .StartMinPosition       = glm::vec3(-5.0f, 5.0f, -35.0f),
                  .StartMaxPosition       = glm::vec3(5.0f, 5.0f, 35.0f),
                  .StartAfterSeconds      = 0.0f,
                  .LifeTimeMin            = 10.0f,
                  .LifeTimeMax            = 20.0f,
                  .StartMinSpeed          = glm::vec3(0.0, 0.1, 0.0),
                  .StartMaxSpeed          = glm::vec3(0.0, 0.3, 0.0),
                  .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
                  .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
                  .StartMinSize           = glm::vec3(0.08),
                  .StartMaxSize           = glm::vec3(0.12),
                  .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                  .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
                  .StartMaxColor          = 0x228B22FF,
                  .StartMinColor          = 0x31D4928C
              },
              GPC_SHADER("Compute Shader Particle")
          );

            EntityID border = CreateEntityAs3D();
            auto transform = GetComponent<Transform3D>(border);
            transform->LocalTransform.AddPosition(70, 14, 35);
            auto emitter = AddComponent<ParticleEmitter>(border);
            emitter->pGeometry = GPC_MESH("Sphere");
            emitter->pComputeProgram = &GPC_COMPUTE_PROGRAM("WorldBorder1 Particle Program")->Program;
            emitter->SetLooping(true);
            emitter->AutoStart();
        }

        GPC_ASSETS->AddParticleProgram(
           "Forge Particle Program",
           {
               .Looping                = false,
               .MaxParticle            = 32,
               .Mode                   = ParticleEmissionMode::OVER_TIME,
               .Shape                  = ParticleEmissionShape::SHPERE,
               .StartMinPosition       = glm::vec3(-0.07f),
               .StartMaxPosition       = glm::vec3(0.12f),
               .StartAfterSeconds      = 0.0f,
               .LifeTimeMin            = 3.0f,
               .LifeTimeMax            = 10.0f,
               .StartMinSpeed          = glm::vec3(0, 0.2, 0),
               .StartMaxSpeed          = glm::vec3(0, 0.7, 0),
               .StartMinRotationSpeed  = Transform::Euler_To_Quaternion(0.0, 0.0, 0.0f, XYZ),
               .StartMaxRotationSpeed  = Transform::Euler_To_Quaternion(glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, glm::pi<float>()/3.0f, XYZ),
               .StartMinSize           = glm::vec3(0.3f),
               .StartMaxSize           = glm::vec3(0.5f),
               .StartMinRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
               .StartMaxRotation       = Transform::YawPitchRoll_To_Quaternion(GPC_PI * 0.25f, GPC_PI * 0.75f, GPC_PI),
               .StartMaxColor          = 0x0C0A097D,
               .StartMinColor          = 0xD3D3D33C
           },
           GPC_SHADER("Compute Shader Particle")
       );
    }

    void SceneVillageV1::SetupSilene()
    {

        auto id = CreateEntityAs3D();
        Transform3D* transform = GetComponent<Transform3D>(id);
        transform->LocalTransform.SetScale({0.003f, 0.003f, 0.003f});
        transform->LocalTransform.RotateYawPitchRoll(glm::radians(-140.0f), 0.0, 0.0f);
        transform->LocalTransform.AddPosition({33.5f, 2.0f, 35.5f});

        auto renderer = AddComponent<Render3DComponent>(id);
        renderer->pGeometry = GPC_MESH("Silene");
        renderer->SetTexture("2048x2048", "SileneTexture");

        mp_Animator = AddComponent<AnimationComponent>(id);
        mp_Animator->SetFbxData(mp_SileneMeshData);
        mp_Animator->StackIndex = 1;
        mp_Animator->Playing = true;
        mp_Animator->Looping = true;
        mp_Animator->Speed = 1.83f;
        mp_Animator->ChangeAnimationTo("mixamo.com");


    }
}
