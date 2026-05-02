#pragma once

#include "FollowTarget.h"
#include "FreeCamera.h"
#include "../Expedition/PlayerController.h"
#include "SceneDefault.h"
#include "TransiFeuillle.h"
#include "Expedition/PlayerInteraction.h"
#include "Map Generation/EnnemiGeneratorBehavior.h"
#include "Map Generation/ResourceGeneratorBehavior.h"
#include "Map Generation/TerrainGeneratorBehavior.h"
#include "Expedition/LivingEntityBehavior.h"
#include "Expedition/BackToVillageBehavior.h"
#include "UI/UiButton.h"
#include "Village/Inventory.h"
#include "UI/UiReactive.h"
#include "UI/UiSignal.h"
#include "UISettingsGame_Behavior.h"
#include "Expedition/IrisFollowing.h"

namespace GPC {
    struct UiCanvas;
    struct UiText;

    struct SceneExpeditionStartCtx {

        // Mission 1 - 16+
        uint32_t MissionDifficulty  = 1;

        Helmet* pHelmet             = nullptr;
        Gloves* pGloves             = nullptr;
        ChestPlate* pChestPlate     = nullptr;
        Legs* pLegs                 = nullptr;
        Boots* pBoots               = nullptr;

        MeleeWeapon* pMelee         = nullptr;
        DistanceWeapon* pDistance   = nullptr;
    };

    class SceneExpedition : public SceneDefault {

        FreeCamera* mp_FreeCameraBehavior = nullptr;
        CameraComponent* mp_FreeCameraCamera = nullptr;

        FollowTarget* mp_MainCameraBehavior = nullptr;

        Transform3D* mp_PlayerTransform = nullptr;
        PlayerController* mp_PlayerControllerBehavior = nullptr;
        PlayerInteraction* mp_PlayerInteractionBehavior = nullptr;
        LivingEntityBehavior* mp_PlayerLivingBehavior = nullptr;

        Transform3D* mp_IrisTransform = nullptr;
        IrisFollowing* mp_IrisBehavior = nullptr;

        TerrainGeneratorBehavior* mp_TerrainGeneratorBehavior = nullptr;
        EnnemiGeneratorBehavior* mp_EnnemiGeneratorBehavior = nullptr;
        ResourceGeneratorBehavior* mp_ResourceGeneratorBehavior = nullptr;

        Transform3D* mp_EndOfMissionTransform = nullptr;

        GENERATION_ID m_GenerationID{0};
        std::vector<RoomMeta> m_RoomMetas;
        UniformTextureID m_UniformTextureIDs{};

        // UI
        UiCanvas* mp_MainUI = nullptr;
        UiCanvas* mp_CharacterUi = nullptr;
        UiCanvas* mp_MaterialUi = nullptr;
        UiCanvas* mp_MaterialCrystalUi = nullptr;

        UiCanvas* mp_PostGameUI = nullptr;
        UiText* mp_ContinueButtonText = nullptr;
        UiText* mp_TitleText = nullptr;
        UiButton* mp_ContinueButton = nullptr;

        UiText* mp_PostGameZoneText = nullptr;
        UiText* mp_PostGameFiberText = nullptr;
        UiText* mp_PostGameWoodText = nullptr;
        UiText* mp_PostGameFoodText = nullptr;
        UiText* mp_PostGameStoneText = nullptr;
        UiText* mp_PostGameOreText = nullptr;
        UiText* mp_PostGameCrystalT1Text = nullptr; // PurpleGemme
        UiText* mp_PostGameCrystalT2Text = nullptr; // RedGemme
        UiText* mp_PostGameCrystalT3Text = nullptr; // BlueGemme
        UiText* mp_PostGameCrystalT4Text = nullptr; // GreenGemme
        UiText* mp_PostGameCrystalT5Text = nullptr; // YellowGemme
        UiText* mp_PostGameCrystalT6Text = nullptr; // OrangeGemme

        UiSprite* mp_PostGameBaseIcons[5] = {};
        UiSprite* mp_PostGameCrystalIcons[6] = {};
        UiSprite* mp_PostGameBtn1Sprite = nullptr;
        UiSprite* mp_PostGameBtn2Sprite = nullptr;
        UiSprite* mp_bgSprite = nullptr;

        UISettings* mp_SettingsBehaviour = nullptr;
        bool settingsIsVisible = false;

        UiButton* mp_CrystalButton = nullptr;

        UiText*   mp_PlayerHpText = nullptr;
        UiText*   mp_IrisHpText   = nullptr;
        UiText*   mp_FiberText    = nullptr;
        UiText*   mp_WoodText     = nullptr;
        UiText*   mp_FoodText     = nullptr;
        UiText*   mp_StoneText    = nullptr;
        UiText*   mp_OreText      = nullptr;
        UiText*   mp_CrystalText   = nullptr;
        UiText*   mp_CrystalT1Text = nullptr;
        UiText*   mp_CrystalT2Text = nullptr;
        UiText*   mp_CrystalT3Text = nullptr;
        UiText*   mp_CrystalT4Text = nullptr;
        UiText*   mp_CrystalT5Text = nullptr;
        UiText*   mp_CrystalT6Text = nullptr;

        glm::vec2 m_MiniMapPosition = { 10.0f, 10.0f };
        float m_MiniMapScale     = 1.5f;
        float m_MiniMapRotation  = 0.0f;
        UiSprite* mp_PlayerMiniMapIndicator = nullptr;
        UiSprite* mp_MiniMap = nullptr;
        UiSprite* mp_MiniMapBackground = nullptr;

        UiReactive<std::string> m_PlayerHpReactive{ "100/100" };
        UiReactive<std::string> m_IrisHpReactive{ "100/100" };
        UiReactive<std::string> m_FiberReactive{ "0" };
        UiReactive<std::string> m_WoodReactive{ "0" };
        UiReactive<std::string> m_FoodReactive{ "0" };
        UiReactive<std::string> m_StoneReactive{ "0" };
        UiReactive<std::string> m_OreReactive{ "0" };
        UiReactive<std::string> m_CrystalReactive{ "0" };
        UiReactive<std::string> m_CrystalT1Reactive{ "0" };
        UiReactive<std::string> m_CrystalT2Reactive{ "0" };
        UiReactive<std::string> m_CrystalT3Reactive{ "0" };
        UiReactive<std::string> m_CrystalT4Reactive{ "0" };
        UiReactive<std::string> m_CrystalT5Reactive{ "0" };
        UiReactive<std::string> m_CrystalT6Reactive{ "0" };

        bool m_AdminPannelIsOpen = false;
        UiSprite* mp_BackgroundAdminPanel = nullptr;

        // Audio
        AudioSource* mp_ExploreMusic = nullptr;
        AudioSource* mp_FightMusic = nullptr;
        AudioSource* mp_UIpopSfx   = nullptr;
        AudioSource* mp_WinSFX    = nullptr;

        RandomAudio m_FootstepPlayer;
        RandomAudio m_SlidePlayer;
        RandomAudio m_MiningStonePlayer;
        RandomAudio m_MiningWoodPlayer;
        RandomAudio m_AttackPlayer;
        RandomAudio m_DamageEnemie;
        RandomAudio m_WalkEnemie;

        // Player Gear :
        Helmet* mp_Helmet = nullptr;

        MeleeWeapon* mp_Melee = nullptr;
        uint32_t m_Difficulty;
        bool m_UseTuto = true;
        float m_StartTime = 10000.0f;

        bool m_MovingTutoHasProc = false;

        Inventory* mp_Inventory;
        UiSignalHandle m_ResourceAddedHandle = 0;

        EntityID BossEID = MAX_ENTITIES;

        FbxMeshData* mp_CaliFbxData;
        FbxMeshData* mp_IrisFbxData;
        FbxMeshData* mp_EnnemiFbxData;

        TransiFeuillle* mp_TransiFeuillle;

        std::array<Batching3DComponent*, PROP_COUNT> mp_Props;

        void UseFreeCamera() const;
        void UseFollowCamera() const;

        void StartMission();
        void DestroyMission();

        void DestroyAllWithAsteroid();

    public:

        void SetupUI();

        void SetupPostGameUI();
        void SetUpAdminPannel();
        void SetupBossBar();
        void ShowPostGameUI(uint32_t zone, bool canContinue = true);

        void UpdateResourceReactive(ResourceID id);

        void SetupAudio();
;
        Inventory* GetInventory() { return mp_Inventory; }
        uint32_t GetDifficulty() { return m_Difficulty; }
        LivingEntityBehavior* GetCali() { return mp_PlayerLivingBehavior; }

        void LoadAllRooms();
        void GenerateMap();
        void GenerateNoise();

        void CreatePlayer();
        void CreateTerrain();
        void CreateEnnemies();
        void CreateResources();
        void CreateEndOfMission();
        void CreateFreeCamera();
        void CreatePlayerCamera();
        void CreateSunLighting();
        void CreateWater();
        void CreateMiniMap();

        void SetPlayerAtSpawnPoint();
        void SetEndOfMissionPoint();
        void SetPlayerAtEnd();

        ErrorType OnAssetsLoad() override;
        void OnAssetsDestroy() override;

        ErrorType OnCreate(SceneInformation &info) override;
        void OnDestroy() override;

        ErrorType OnStart(SceneChangeCtx &ctx) override;
        ErrorType OnEnd() override;

        void OnSceneUpdate() override;
    };

} // GPC