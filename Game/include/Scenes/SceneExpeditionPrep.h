#pragma once

#include <array>
#include <string>
#include <vector>

#include "SceneDefault.h"
#include "UISettingsGame_Behavior.h"
#include "UI/UiBuilder.h"
#include "UI/UiReactive.h"
#include "Expedition/Equipment.h"
#include "Expedition/TutoBehaviors.h"
#include "Village/Inventory.h"

namespace GPC
{
    struct SceneExpeditionPrepCtx
    {
        Inventory* pVillageInventory = nullptr;
    };

    class SceneExpeditionPrep : public SceneDefault
    {
    public:
        ErrorType OnAssetsLoad() override;
        ErrorType OnCreate(SceneInformation& info) override;
        ErrorType OnStart(SceneChangeCtx& ctx) override;
        ErrorType OnEnd() override;
        void OnSceneUpdate() override;
        void OnDestroy() override;

    private:
        enum class PrepPage : uint8_t
        {
            ZoneSelect,
            Preparation
        };

        enum class ActiveHero : uint8_t
        {
            Cali,
            Iris
        };

        struct HeroLoadout
        {
            Helmet*         pHelmet     = nullptr;
            Gloves*         pGloves     = nullptr;
            ChestPlate*     pChestPlate = nullptr;
            Legs*           pLegs       = nullptr;
            Boots*          pBoots      = nullptr;
            MeleeWeapon*    pMelee      = nullptr;
            DistanceWeapon* pDistance   = nullptr;
        };

        static constexpr uint32_t GRID_SLOTS        = 8;
        static constexpr uint32_t SILHOUETTE_SLOTS  = 6;
        static constexpr uint32_t ZONE_BUTTONS      = 4;

        enum SilhouetteSlot : uint8_t
        {
            SLOT_HELMET     = 0,
            SLOT_CHESTPLATE = 1,
            SLOT_GLOVES     = 2,
            SLOT_MELEE      = 3,
            SLOT_LEGS       = 4,
            SLOT_BOOTS      = 5
        };

        PrepPage    m_Page                  = PrepPage::ZoneSelect;
        ActiveHero  m_ActiveHero            = ActiveHero::Cali;
        uint32_t    m_SelectedDifficulty    = 1;
        HeroLoadout m_CaliLoadout           = {};
        HeroLoadout m_IrisLoadout           = {};
        Inventory*  mp_VillageInventory     = nullptr;
        bool        m_UIBuilt               = false;

        std::vector<Equipment*> m_GridItems = {};

        UiCanvas*   mp_MainUI           = nullptr;
        UiCanvas*   mp_ZoneSelectUI     = nullptr;
        UiCanvas*   mp_PreparationUI    = nullptr;

        UiCanvas*   mp_LaunchingScreen      = nullptr;
        UiSprite*   mp_LaunchingBackground  = nullptr;
        UiText*     mp_LaunchingText        = nullptr;
        bool        m_LaunchInProgress      = false;

        UISettings* mp_SettingsBehaviour = nullptr;
        bool settingsIsVisible = false;

        bool m_IsInTutorial              = true;

        UiText*     mp_ActiveHeroLabel   = nullptr;
        UiSprite*   mp_MeleeAxeOverlay   = nullptr;
        UiSprite*   mp_HelmetOverlay     = nullptr;
        UiSprite*   mp_ChestPlateOverlay = nullptr;
        UiSprite*   mp_BottomOverlay     = nullptr;
        UiSprite*   mp_BootsOverlay      = nullptr;
        UiSprite*   mp_CaliPortrait      = nullptr;
        UiSprite*   mp_IrisPortrait      = nullptr;
        UiSprite*   mp_SilhouetteAvatar  = nullptr;
        UiButton*   mp_BackBtn           = nullptr;
        UiButton*   mp_BackBtn2           = nullptr;
        UiButton*   mp_LaunchBtn         = nullptr;
        UiText*     mp_EndlessButtonText      = nullptr;
        bool                                    m_HasStartedTutorial = false;
        std::array<UiButton*, ZONE_BUTTONS>     mp_ZoneButtons       = {};
        std::array<UiText*,   SILHOUETTE_SLOTS> mp_SilhouetteLabels  = {};
        std::array<UiSprite*, SILHOUETTE_SLOTS> mp_SilhouetteSprites = {};
        std::array<UiSprite*, GRID_SLOTS>       mp_GridSprites       = {};
        std::array<UiButton*, GRID_SLOTS>       mp_GridButtons       = {};
        std::array<UiSprite*, ZONE_BUTTONS>     mp_ZoneSprites       = {};

        // ── Tooltip hover (apparait a gauche du slot survole)
        static constexpr uint32_t TOOLTIP_STAT_LINES = 5;
        UiCanvas*   mp_PrepTooltipCanvas = nullptr;
        UiSprite*   mp_PrepTooltipBg     = nullptr;
        UiText*     mp_TooltipTitleLabel = nullptr;
        UiText*     mp_TooltipForgeHint  = nullptr;
        std::array<UiText*, TOOLTIP_STAT_LINES> mp_TooltipStatLabels = {};

        UiReactive<std::string> m_TooltipTitle{ "" };
        std::array<UiReactive<std::string>, TOOLTIP_STAT_LINES> m_TooltipStatLines = {
            UiReactive<std::string>{""},
            UiReactive<std::string>{""},
            UiReactive<std::string>{""},
            UiReactive<std::string>{""},
            UiReactive<std::string>{""}
        };

        LaunchExpeditionTuto* mp_ExpeditionTuto = nullptr;

        void BeginTuto();
        void LoadUIElements();
        void SetupUI();
        void SetupZoneSelectUI(EntityID parent);
        void SetupPreparationUI(EntityID parent);
        void SetupSettingsUI(EntityID parent);

        UiSprite* BuildZoneButton(EntityID parent, uint32_t zoneIndex, const std::string& label, uint32_t difficulty, float offsetY);
        void BuildHeroPortrait(EntityID parent, ActiveHero hero, float offsetX);
        void BuildSilhouetteSlot(EntityID parent, SilhouetteSlot slot, const std::string& spriteIdle, glm::vec2 position);
        void BuildEquipmentGrid(EntityID parent);
        void BuildGridSlot(EntityID parent, uint32_t index, const std::string& sprite);

        void SwitchPage(PrepPage page);
        void SelectZone(uint32_t difficulty);
        void SelectHero(ActiveHero hero);
        void EquipSlotItem(uint32_t slotIndex);
        void RefreshSilhouette();
        void RefreshGridVisual();
        void RefreshZoneSelection();
        void RefreshActiveHeroLabel();
        void RefreshHeroSelection();
        void OnLaunchExpedition();
        void OnBackToVillage();

        void SetupLaunchingScreenUI();

        HeroLoadout& ActiveLoadout();
        void EnsureGridItems();
        void EnsurePoolItems();
        void RebuildGridForActiveHero();
        void ResetLoadouts();
        void SetHierarchyEnabled(EntityID root, bool enabled);

        static OwnerCharacter ActiveHeroToOwner(ActiveHero hero);
        Equipment* FindFirstByOwner(OwnerCharacter owner);

        void BuildPrepTooltip(EntityID parent);
        void ShowTooltipForSlot(uint32_t slotIndex);
        void HideTooltip();
        static std::string EquipmentDisplayName(Equipment* pEq);
        static std::string FormatStat(const BaseStat& stat);
    };
}
