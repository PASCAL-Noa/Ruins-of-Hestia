#include "Scenes/SceneExpeditionPrep.h"

#include "Scenes/SceneExpedition.h"
#include "Scenes/SceneVillageV1.h"
#include "SceneManager.h"
#include "SerialFile.h"
#include "SerialTree.h"
#include "UI/UiButton.h"
#include "UI/UiCanvas.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"
#include "SettingsGame.h"
#include "UI/UiAnim.h"
#include "UI/UiAnimFluent.h"
#include "UiConstants.h"
#include "UI/UiTheme.h"
#include "Expedition/TutoBehaviors.h"
#include "Village/Save.h"

namespace GPC
{
    ErrorType SceneExpeditionPrep::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();
        LoadUIElements();
        return ErrorType::SUCCESS;
    }

    void SceneExpeditionPrep::BeginTuto()
    {
        if (m_HasStartedTutorial) return;
        m_HasStartedTutorial = true;

        auto entity = CreateEntityAs2D();
        mp_ExpeditionTuto = AddBehavior<LaunchExpeditionTuto>(entity);

        mp_ExpeditionTuto->Init(mp_ZoneButtons[1], mp_LaunchBtn, mp_EndlessButtonText, &m_IsInTutorial);
    }

    void SceneExpeditionPrep::LoadUIElements()
    {
        GPC_ASSETS->AddTexture("Prep_Frame_1",       "Resources/UI/PREPARATION/260x227/ui_prep_frame_1.png");
        GPC_ASSETS->AddTexture("Prep_Frame_2",       "Resources/UI/PREPARATION/260x227/ui_prep_frame_2.png");
        GPC_ASSETS->AddTexture("Prep_Expedition",    "Resources/UI/PREPARATION/260x227/ui_prep_expedition.png");
        GPC_ASSETS->AddTexture("Prep_Head",          "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_head.png");
        GPC_ASSETS->AddTexture("Prep_Head_Sel",      "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_head_selected.png");
        GPC_ASSETS->AddTexture("Prep_Body",          "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_body.png");
        GPC_ASSETS->AddTexture("Prep_Body_Sel",      "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_body_selected.png");
        GPC_ASSETS->AddTexture("Prep_Left",          "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_left.png");
        GPC_ASSETS->AddTexture("Prep_Left_Sel",      "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_left_selected.png");
        GPC_ASSETS->AddTexture("Prep_Right",         "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_right.png");
        GPC_ASSETS->AddTexture("Prep_Right_Sel",     "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_right_selected.png");
        GPC_ASSETS->AddTexture("Prep_Feet",          "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_feet.png");
        GPC_ASSETS->AddTexture("Prep_Feet_Sel",      "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_feet_selected.png");
        GPC_ASSETS->AddTexture("Prep_Axe",           "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_axe.png");
        GPC_ASSETS->AddTexture("Prep_Helmet", "Resources/UI/PREPARATION/armor-20260427T183409Z-3-001/armor/_0000_Calque-4.png");
        GPC_ASSETS->AddTexture("Prep_Bottom", "Resources/UI/PREPARATION/armor-20260427T183409Z-3-001/armor/_0001_bottom.png");
        GPC_ASSETS->AddTexture("Prep_Boots", "Resources/UI/PREPARATION/armor-20260427T183409Z-3-001/armor/_0002_boots.png");
        GPC_ASSETS->AddTexture("Prep_ChestPlate", "Resources/UI/PREPARATION/armor-20260427T183409Z-3-001/armor/_0003_torso.png");
        GPC_ASSETS->AddTexture("Prep_CaliWeapon",    "Resources/UI/PREPARATION/260x227/EQUIPPED/ui_prep_gant.png");

        GPC_ASSETS->AddTextureArray(
            "260x227",
            260, 227,
            {
                GPC_TEXTURE("Prep_Frame_1"),
                GPC_TEXTURE("Prep_Frame_2"),
                GPC_TEXTURE("Prep_Expedition"),
                GPC_TEXTURE("Prep_Head"),
                GPC_TEXTURE("Prep_Head_Sel"),
                GPC_TEXTURE("Prep_Body"),
                GPC_TEXTURE("Prep_Body_Sel"),
                GPC_TEXTURE("Prep_Left"),
                GPC_TEXTURE("Prep_Left_Sel"),
                GPC_TEXTURE("Prep_Right"),
                GPC_TEXTURE("Prep_Right_Sel"),
                GPC_TEXTURE("Prep_Feet"),
                GPC_TEXTURE("Prep_Feet_Sel"),
                GPC_TEXTURE("Prep_Axe"),
                GPC_TEXTURE("Prep_CaliWeapon"),
                GPC_TEXTURE("Prep_Helmet"),
                GPC_TEXTURE("Prep_Bottom"),
                GPC_TEXTURE("Prep_Boots"),
                GPC_TEXTURE("Prep_ChestPlate")
            }
        );

        GPC_ASSETS->AddTexture("Prep_Avatar_Iris", "Resources/UI/PREPARATION/305x652/ui_avatar.png");
        GPC_ASSETS->AddTexture("Prep_Avatar_Cali", "Resources/UI/PREPARATION/305x652/cali de face.png");
        GPC_ASSETS->AddTextureArray(
            "305x652",
            305, 652,
            {
                GPC_TEXTURE("Prep_Avatar_Iris"),
                GPC_TEXTURE("Prep_Avatar_Cali")
            }
        );

        GPC_ASSETS->AddTexture("Prep_Title_Bg",       "Resources/UI/PREPARATION/613x161/ui_preparation.png");
        GPC_ASSETS->AddTexture("Prep_Inv_2",          "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_2.png");
        GPC_ASSETS->AddTexture("Prep_Inv_3",          "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_3.png");
        GPC_ASSETS->AddTexture("Prep_Inv_4",          "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_4.png");
        GPC_ASSETS->AddTexture("Prep_Inv_5",          "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_5.png");
        GPC_ASSETS->AddTexture("Prep_Inv_6",          "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_6.png");
        GPC_ASSETS->AddTexture("Prep_Inv_7",          "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_7.png");
        GPC_ASSETS->AddTexture("Prep_Inv_8",          "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_8.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Axe",        "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_axe.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Axe_Sel",    "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_axe_selected.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Boots",    "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_boots.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Boots_Sel",    "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_boots_selected.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Bottom",    "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_bottom.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Bottom_Sel",    "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_bottom_selected.png");
        GPC_ASSETS->AddTexture("Prep_Inv_ChestPlate",    "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_chest.png");
        GPC_ASSETS->AddTexture("Prep_Inv_ChestPlate_Sel",    "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_chest_selected.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Helmet",    "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_helmet.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Helmet_Sel",    "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_helmet_selected.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Gauntlet",      "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_gauntlet.png");
        GPC_ASSETS->AddTexture("Prep_Inv_Gauntlet_Sel",  "Resources/UI/PREPARATION/613x161/INVENTORY/ui_button_prep_inventory_gauntlet_selected.png");

        GPC_ASSETS->AddTextureArray(
            "613x161",
            613, 161,
            {
                GPC_TEXTURE("Prep_Title_Bg"),
                GPC_TEXTURE("Prep_Inv_2"),
                GPC_TEXTURE("Prep_Inv_3"),
                GPC_TEXTURE("Prep_Inv_4"),
                GPC_TEXTURE("Prep_Inv_5"),
                GPC_TEXTURE("Prep_Inv_6"),
                GPC_TEXTURE("Prep_Inv_7"),
                GPC_TEXTURE("Prep_Inv_8"),
                GPC_TEXTURE("Prep_Inv_Axe"),
                GPC_TEXTURE("Prep_Inv_Axe_Sel"),
                GPC_TEXTURE("Prep_Inv_Boots"),
                GPC_TEXTURE("Prep_Inv_Boots_Sel"),
                GPC_TEXTURE("Prep_Inv_Bottom"),
                GPC_TEXTURE("Prep_Inv_Bottom_Sel"),
                GPC_TEXTURE("Prep_Inv_ChestPlate"),
                GPC_TEXTURE("Prep_Inv_ChestPlate_Sel"),
                GPC_TEXTURE("Prep_Inv_Helmet"),
                GPC_TEXTURE("Prep_Inv_Helmet_Sel"),
                GPC_TEXTURE("Prep_Inv_Gauntlet"),
                GPC_TEXTURE("Prep_Inv_Gauntlet_Sel"),
            }
        );

        GPC_ASSETS->AddTexture("ANIM00_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00000.png");
        GPC_ASSETS->AddTexture("ANIM01_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00001.png");
        GPC_ASSETS->AddTexture("ANIM02_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00002.png");
        GPC_ASSETS->AddTexture("ANIM03_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00003.png");
        GPC_ASSETS->AddTexture("ANIM04_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00004.png");
        GPC_ASSETS->AddTexture("ANIM05_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00005.png");
        GPC_ASSETS->AddTexture("ANIM06_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00006.png");
        GPC_ASSETS->AddTexture("ANIM07_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00007.png");
        GPC_ASSETS->AddTexture("ANIM08_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00008.png");
        GPC_ASSETS->AddTexture("ANIM09_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00009.png");
        GPC_ASSETS->AddTexture("ANIM10_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00010.png");
        GPC_ASSETS->AddTexture("ANIM11_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00011.png");
        GPC_ASSETS->AddTexture("ANIM12_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00012.png");
        GPC_ASSETS->AddTexture("ANIM13_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00013.png");
        GPC_ASSETS->AddTexture("ANIM14_Prep_Title_Bg", "Resources/UI/PREPARATION/613x161/ANIM/ui_preparation/ui_preparation_00014.png");
        GPC_ASSETS->AddTextureArray(
            "ANIMATION_Prep",
            613, 161,
            {
                GPC_TEXTURE("ANIM00_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM01_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM02_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM03_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM04_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM05_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM06_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM07_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM08_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM09_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM10_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM11_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM12_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM13_Prep_Title_Bg"),
                GPC_TEXTURE("ANIM14_Prep_Title_Bg")
            }
        );

        GPC_ASSETS->AddTexture("Prep_Tooltip", "Resources/UI/PREPARATION/637x243/ui_tooltip_mouse.png");
        GPC_ASSETS->AddTextureArray(
            "637x243",
            637, 243,
            { GPC_TEXTURE("Prep_Tooltip") }
        );

        GPC_ASSETS->AddTexture("Exp_Banner", "Resources/UI/EXPEDITION/452x151/ui_expedition.png");
        GPC_ASSETS->AddTextureArray(
            "452x151",
            452, 151,
            { GPC_TEXTURE("Exp_Banner") }
        );

        GPC_ASSETS->AddTexture("ANIM00_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00000.png");
        GPC_ASSETS->AddTexture("ANIM01_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00001.png");
        GPC_ASSETS->AddTexture("ANIM02_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00002.png");
        GPC_ASSETS->AddTexture("ANIM03_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00003.png");
        GPC_ASSETS->AddTexture("ANIM04_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00004.png");
        GPC_ASSETS->AddTexture("ANIM05_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00005.png");
        GPC_ASSETS->AddTexture("ANIM06_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00006.png");
        GPC_ASSETS->AddTexture("ANIM07_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00007.png");
        GPC_ASSETS->AddTexture("ANIM08_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00008.png");
        GPC_ASSETS->AddTexture("ANIM09_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00009.png");
        GPC_ASSETS->AddTexture("ANIM10_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00010.png");
        GPC_ASSETS->AddTexture("ANIM11_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00011.png");
        GPC_ASSETS->AddTexture("ANIM12_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00012.png");
        GPC_ASSETS->AddTexture("ANIM13_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00013.png");
        GPC_ASSETS->AddTexture("ANIM14_Exp_Banner", "Resources/UI/EXPEDITION/452x151/ANIM/ui_expedition/ui_expedition_00014.png");
        GPC_ASSETS->AddTextureArray(
            "ANIMATION_Exp_Banner",
            452, 151,
            {
                GPC_TEXTURE("ANIM00_Exp_Banner"),
                GPC_TEXTURE("ANIM01_Exp_Banner"),
                GPC_TEXTURE("ANIM02_Exp_Banner"),
                GPC_TEXTURE("ANIM03_Exp_Banner"),
                GPC_TEXTURE("ANIM04_Exp_Banner"),
                GPC_TEXTURE("ANIM05_Exp_Banner"),
                GPC_TEXTURE("ANIM06_Exp_Banner"),
                GPC_TEXTURE("ANIM07_Exp_Banner"),
                GPC_TEXTURE("ANIM08_Exp_Banner"),
                GPC_TEXTURE("ANIM09_Exp_Banner"),
                GPC_TEXTURE("ANIM10_Exp_Banner"),
                GPC_TEXTURE("ANIM11_Exp_Banner"),
                GPC_TEXTURE("ANIM12_Exp_Banner"),
                GPC_TEXTURE("ANIM13_Exp_Banner"),
                GPC_TEXTURE("ANIM14_Exp_Banner"),
            }
        );

        GPC_ASSETS->AddTexture("Exp_Zone_1", "Resources/UI/EXPEDITION/703x145/ui_button_zone_select_1.png");
        GPC_ASSETS->AddTexture("Exp_Zone_2", "Resources/UI/EXPEDITION/703x145/ui_button_zone_select_2.png");
        GPC_ASSETS->AddTexture("Exp_Zone_3", "Resources/UI/EXPEDITION/703x145/ui_button_zone_select_3.png");
        GPC_ASSETS->AddTexture("Exp_Zone_4", "Resources/UI/EXPEDITION/703x145/ui_button_zone_select_4.png");
        GPC_ASSETS->AddTexture("Exp_Zone_4_brown", "Resources/UI/EXPEDITION/703x145/ui_button_zone_select_4_brown.png");
        GPC_ASSETS->AddTextureArray(
            "703x145",
            703, 145,
            {
                GPC_TEXTURE("Exp_Zone_1"),
                GPC_TEXTURE("Exp_Zone_2"),
                GPC_TEXTURE("Exp_Zone_3"),
                GPC_TEXTURE("Exp_Zone_4"),
                GPC_TEXTURE("Exp_Zone_4_brown")
            }
        );

         // ANIMATION BUTTON
        {
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM00", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00000.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM01", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00001.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM02", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00002.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM03", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00003.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM04", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00004.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM05", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00005.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM06", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00006.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM07", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00007.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM08", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00008.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM09", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00009.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM10", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00010.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM11", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00011.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM12", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00012.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM13", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00013.png");
            GPC_ASSETS->AddTexture("Exp_Zone_1_ANIM14", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_1/ui_button_zone_select_hover_1_00014.png");
            GPC_ASSETS->AddTextureArray(
                "ANIMATION_Exp_Zone_1",
                723, 165,
                {
                    GPC_TEXTURE("Exp_Zone_1_ANIM00"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM01"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM02"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM03"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM04"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM05"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM06"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM07"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM08"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM09"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM10"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM11"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM12"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM13"),
                    GPC_TEXTURE("Exp_Zone_1_ANIM14"),
                }
            );

            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM00", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00000.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM01", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00001.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM02", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00002.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM03", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00003.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM04", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00004.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM05", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00005.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM06", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00006.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM07", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00007.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM08", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00008.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM09", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00009.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM10", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00010.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM11", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00011.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM12", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00012.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM13", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00013.png");
            GPC_ASSETS->AddTexture("Exp_Zone_2_ANIM14", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_2/ui_button_zone_select_hover_2_00014.png");
            GPC_ASSETS->AddTextureArray(
                "ANIMATION_Exp_Zone_2",
                723, 165,
                {
                    GPC_TEXTURE("Exp_Zone_2_ANIM00"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM01"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM02"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM03"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM04"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM05"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM06"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM07"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM08"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM09"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM10"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM11"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM12"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM13"),
                    GPC_TEXTURE("Exp_Zone_2_ANIM14"),
                }
            );

            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM00", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00000.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM01", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00001.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM02", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00002.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM03", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00003.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM04", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00004.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM05", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00005.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM06", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00006.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM07", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00007.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM08", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00008.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM09", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00009.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM10", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00010.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM11", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00011.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM12", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00012.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM13", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00013.png");
            GPC_ASSETS->AddTexture("Exp_Zone_3_ANIM14", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_3/ui_button_zone_select_hover_3_00014.png");
            GPC_ASSETS->AddTextureArray(
                "ANIMATION_Exp_Zone_3",
                723, 165,
                {
                    GPC_TEXTURE("Exp_Zone_3_ANIM00"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM01"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM02"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM03"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM04"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM05"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM06"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM07"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM08"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM09"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM10"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM11"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM12"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM13"),
                    GPC_TEXTURE("Exp_Zone_3_ANIM14"),
                }
            );

            GPC_ASSETS->AddTexture("Nerd_Silene_Right",      "Resources/UI/VILLAGE/461x352/point droit.png");
            GPC_ASSETS->AddTexture("Nerd_Silene_Left",       "Resources/UI/VILLAGE/461x352/point gauche.png");

            GPC_ASSETS->AddTextureArray(
               "NerdSilene",
               461, 352,
               {
                   GPC_TEXTURE("Nerd_Silene_Right"),
                   GPC_TEXTURE("Nerd_Silene_Left")
               });

            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM00", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00000.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM01", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00001.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM02", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00002.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM03", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00003.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM04", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00004.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM05", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00005.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM06", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00006.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM07", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00007.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM08", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00008.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM09", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00009.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM10", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00010.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM11", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00011.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM12", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00012.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM13", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00013.png");
            GPC_ASSETS->AddTexture("Exp_Zone_4_ANIM14", "Resources/UI/EXPEDITION/723x165/ANIM/ui_button_zone_select_hover_4/ui_button_zone_select_hover_4_00014.png");
            GPC_ASSETS->AddTextureArray(
                "ANIMATION_Exp_Zone_4",
                723, 165,
                {
                    GPC_TEXTURE("Exp_Zone_4_ANIM00"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM01"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM02"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM03"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM04"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM05"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM06"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM07"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM08"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM09"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM10"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM11"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM12"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM13"),
                    GPC_TEXTURE("Exp_Zone_4_ANIM14"),
                }
            );
        }

        GPC_ASSETS->AddTexture("Exp_Panel_Bg", "Resources/UI/EXPEDITION/993x1080/ui_zone_select.png");
        GPC_ASSETS->AddTextureArray(
            "993x1080",
            993, 1080,
            { GPC_TEXTURE("Exp_Panel_Bg") }
        );

        GPC_ASSETS->AddTexture("ANIM00_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00000.png");
        GPC_ASSETS->AddTexture("ANIM01_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00001.png");
        GPC_ASSETS->AddTexture("ANIM02_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00002.png");
        GPC_ASSETS->AddTexture("ANIM03_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00003.png");
        GPC_ASSETS->AddTexture("ANIM04_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00004.png");
        GPC_ASSETS->AddTexture("ANIM05_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00005.png");
        GPC_ASSETS->AddTexture("ANIM06_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00006.png");
        GPC_ASSETS->AddTexture("ANIM07_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00007.png");
        GPC_ASSETS->AddTexture("ANIM08_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00008.png");
        GPC_ASSETS->AddTexture("ANIM09_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00009.png");
        GPC_ASSETS->AddTexture("ANIM10_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00010.png");
        GPC_ASSETS->AddTexture("ANIM11_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00011.png");
        GPC_ASSETS->AddTexture("ANIM12_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00012.png");
        GPC_ASSETS->AddTexture("ANIM13_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00013.png");
        GPC_ASSETS->AddTexture("ANIM14_Exp_Panel", "Resources/UI/EXPEDITION/993x1080/ANIM/ui_zone_select/ui_zone_select_00014.png");
        GPC_ASSETS->AddTextureArray(
                    "ANIMATION_Exp_Panel",
                    993, 1080,
                    {
                        GPC_TEXTURE("ANIM00_Exp_Panel"),
                        GPC_TEXTURE("ANIM01_Exp_Panel"),
                        GPC_TEXTURE("ANIM02_Exp_Panel"),
                        GPC_TEXTURE("ANIM03_Exp_Panel"),
                        GPC_TEXTURE("ANIM04_Exp_Panel"),
                        GPC_TEXTURE("ANIM05_Exp_Panel"),
                        GPC_TEXTURE("ANIM06_Exp_Panel"),
                        GPC_TEXTURE("ANIM07_Exp_Panel"),
                        GPC_TEXTURE("ANIM08_Exp_Panel"),
                        GPC_TEXTURE("ANIM09_Exp_Panel"),
                        GPC_TEXTURE("ANIM10_Exp_Panel"),
                        GPC_TEXTURE("ANIM11_Exp_Panel"),
                        GPC_TEXTURE("ANIM12_Exp_Panel"),
                        GPC_TEXTURE("ANIM13_Exp_Panel"),
                        GPC_TEXTURE("ANIM14_Exp_Panel"),
                    }
                );

        GPC_ASSETS->AddTexture("Map_Background", "Resources/UI/EXPEDITION/map/map ui.PNG");
        GPC_ASSETS->AddTexture("Map_Background_Blur", "Resources/UI/EXPEDITION/map/map ui blur.PNG");
        GPC_ASSETS->AddTextureArray(
            "Map",
            1920, 1080,
            {
                GPC_TEXTURE("Map_Background"),
                GPC_TEXTURE("Map_Background_Blur")
            }
        );

        GPC_ASSETS->AddTexture("Exp_Map_Point",        "Resources/UI/EXPEDITION/152x216/ui_map_point.png");
        GPC_ASSETS->AddTexture("Exp_Map_Point_Button", "Resources/UI/EXPEDITION/152x216/ui_button_map_point.png");
        GPC_ASSETS->AddTextureArray(
            "152x216",
            152, 216,
            {
                GPC_TEXTURE("Exp_Map_Point"),
                GPC_TEXTURE("Exp_Map_Point_Button")
            }
        );
    }

    ErrorType SceneExpeditionPrep::OnCreate(SceneInformation& info)
    {
        SceneDefault::OnCreate(info);
        SettingsGame::Get().Load();
        CreateDebugOverlay();

        m_IsInTutorial = true;

        return ErrorType::SUCCESS;
    }

    ErrorType SceneExpeditionPrep::OnStart(SceneChangeCtx& ctx)
    {
        SceneDefault::OnStart(ctx);

        SceneExpeditionPrepCtx* pCtx = static_cast<SceneExpeditionPrepCtx*>(ctx.pData);
        if (pCtx != nullptr)
        {
            mp_VillageInventory = pCtx->pVillageInventory;
        }

        EnsureGridItems();

        if (!m_UIBuilt)
        {
            auto settingsEntity = CreateEntityAs2D();
            mp_SettingsBehaviour = AddBehavior<UISettings>(settingsEntity);

            SetupUI();
            SetupLaunchingScreenUI();
            m_UIBuilt = true;
        }

        m_LaunchInProgress = false;
        if (mp_LaunchingScreen)
        {
            UiAnim::Cancel(*this, mp_LaunchingScreen->GetEntityID());
            if (mp_LaunchingBackground) mp_LaunchingBackground->SetTint(Color(0, 0, 0, 0));
            if (mp_LaunchingText)       mp_LaunchingText->SetTint(Color(255, 255, 255, 0));
            SetHierarchyEnabled(mp_LaunchingScreen->GetEntityID(), false);
        }

        ResetLoadouts();
        m_SelectedDifficulty = 1;
        m_ActiveHero = ActiveHero::Cali;
        m_Page = PrepPage::ZoneSelect;

        RefreshActiveHeroLabel();
        RefreshHeroSelection();
        RefreshSilhouette();
        RefreshGridVisual();
        RefreshZoneSelection();

        if (mp_ZoneSelectUI)   SetHierarchyEnabled(mp_ZoneSelectUI->GetEntityID(),  true);
        if (mp_PreparationUI)  SetHierarchyEnabled(mp_PreparationUI->GetEntityID(), false);
        if (settingsIsVisible) SetHierarchyEnabled(mp_SettingsBehaviour->GetEntityID(),    false);

        return ErrorType::SUCCESS;
    }

    ErrorType SceneExpeditionPrep::OnEnd()
    {
        SceneDefault::OnEnd();
        m_GridItems.clear();
        return ErrorType::SUCCESS;
    }

    void SceneExpeditionPrep::OnSceneUpdate()
    {
        SceneDefault::OnSceneUpdate();


        SerialTree tree;
        // TODO : Tuto desactive pour tester inventire -> reactiver = enelev "false &&"
        if (SerialFile::Load(Saves::SAVE_FILE_PATH, tree) != ErrorType::SUCCESS)
        {
            if (m_IsInTutorial && !m_HasStartedTutorial)
            {
                BeginTuto();
            }
        }
    }

    void SceneExpeditionPrep::OnDestroy()
    {
        m_GridItems.clear();

        mp_PrepTooltipCanvas = nullptr;
        mp_PrepTooltipBg     = nullptr;
        mp_TooltipTitleLabel = nullptr;
        mp_TooltipForgeHint  = nullptr;
        mp_TooltipStatLabels.fill(nullptr);
        mp_GridButtons.fill(nullptr);
    }

    void SceneExpeditionPrep::SetupUI()
    {
        UiBuilder b(this);

        mp_MainUI = b.At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                     .Size({ 1080.0f, 720.0f })
                     .Tint(Colors::TRANSPARENT)
                     .FitToScreen(true)
                     .BuildCanvas();

        if (!mp_MainUI) return;

        SetupZoneSelectUI(mp_MainUI->GetEntityID());
        SetupPreparationUI(mp_MainUI->GetEntityID());
        SetupSettingsUI(mp_MainUI->GetEntityID());
    }

    void SceneExpeditionPrep::SetupZoneSelectUI(EntityID parent)
    {
        UiBuilder b(this);

        mp_ZoneSelectUI = b.ChildOf(parent)
                        .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                        .Size({ 1080.0f, 720.0f })
                        .Tint(Colors::FIREBRICK)
                        .FitToScreen(true)
                        .BuildCanvas();

        if (!mp_ZoneSelectUI) return;

        const EntityID panel = mp_ZoneSelectUI->GetEntityID();

         b.ChildOf(panel)
                 .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                 .Sprite("Map", "Map_Background")
                 .BuildSprite();

        b.ChildOf(panel)
            .At(Anchors::MIDDLE_LEFT, { 200.0f, 100.0f })
            .Scale({0.5f,0.5f})
            .Sprite("152x216","Exp_Map_Point" )
            .BuildSprite();

        b.ChildOf(panel)
            .At(Anchors::MIDDLE_LEFT, { 200.0f, 200.0f })
            .Scale({0.5f,0.5f})
            .Sprite("152x216","Exp_Map_Point_Button" )
            .BuildSprite();

        UiSprite* Background_Selector = b.ChildOf(panel)
                                     .At(Anchors::TOP_RIGHT, { 0.0f, 0.0f })
                                     .Sprite("993x1080", "Exp_Panel_Bg")
                                     .Scale({ 0.55f, 0.67f })
                                     .BuildSprite();

        if (!Background_Selector) return;

        UiAnim::On(*this, Background_Selector)
              .FadeTo(1.0f, 5.0f)
              .OnUpdate([Background_Selector](float t) {

                  int frameNum = static_cast<int>(t * 14.9f);
                  std::string frameName = "ANIM";
                  if (frameNum < 10) frameName += "0";
                  frameName += std::to_string(frameNum) + "_Exp_Panel";

                  Background_Selector->SetSprite("ANIMATION_Exp_Panel", frameName);
              })
              .Repeat(-1)
              .Play();

        b.ChildOf(panel)
            .At(Anchors::TOP_RIGHT, { -150.0f, 70.0f })
            .FontSize(55.0f)
            .Tint(Colors::PERFECT_BROWN)
            .Text("PERSEPHIE")
            .BuildText();

        UiSprite* banner = b.ChildOf(panel)
                            .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                            .Sprite("452x151", "Exp_Banner")
                            .Scale({ 0.5f, 0.5f })
                            .BuildSprite();

        if (banner)
        {
            b.ChildOf(banner->GetEntityID())
             .At(Anchors::MIDDLE_LEFT, { 15.0f, -15.0f })
             .FontSize(35.0f)
             .Tint(Colors::PERFECT_BROWN)
             .Text("EXPEDITION")
             .BuildText();

            UiAnim::On(*this, banner)
               .FadeTo(1.0f, 5.0f)
               .OnUpdate([banner](float t) {

                   int frameNum = static_cast<int>(t * 14.9f);
                   std::string frameName = "ANIM";
                   if (frameNum < 10) frameName += "0";
                   frameName += std::to_string(frameNum) + "_Exp_Banner";

                   banner->SetSprite("ANIMATION_Exp_Banner", frameName);
               })
               .Repeat(-1)
               .Play();
        }

        BuildZoneButton(panel, 1, "ZONE 1", 1, 190.0f);
        BuildZoneButton(panel, 2, "ZONE 2",   2,  310.0f);
        BuildZoneButton(panel, 3, "ZONE 3",   3,   430.0f);
        UiSprite* endlessSprite = BuildZoneButton(panel, 0, "ENDLESS",   99,  550.0f);

        UiSprite* backBtn = b.ChildOf(panel)
                             .At(Anchors::BOTTOM_LEFT, { 20.0f, -20.0f })
                             .Sprite("218x169", "Button_Delete")
                             .Scale({ 0.45f, 0.45f })
                             .BuildSprite();

        if (backBtn)
        {
            if (mp_BackBtn = b.BuildButton(backBtn))
            {
                mp_BackBtn->OnClick.ConnectLambda([this]()
                {
                    OnBackToVillage();
                });
            }
        }
    }

    UiSprite* SceneExpeditionPrep::BuildZoneButton(EntityID parent, uint32_t zoneIndex, const std::string& label, uint32_t difficulty, float offsetY)
    {
        UiBuilder b(this);

        const std::string spriteName = "Exp_Zone_" + std::to_string(zoneIndex + 1);

        UiSprite* btnSprite = nullptr;

        if (zoneIndex == 0 || zoneIndex > 1)
        {
            btnSprite = b.ChildOf(parent)
                       .At(Anchors::TOP_RIGHT, { -70.0f, offsetY })
                       .Sprite("703x145", "Exp_Zone_4_brown")
                       .Scale({ 0.6f, 0.6f })
                       .Tint(Colors::PERFECT_BROWN)
                       .BuildSprite();

            if (!btnSprite) return nullptr;
        }
        else
        {
            btnSprite = b.ChildOf(parent)
                      .At(Anchors::TOP_RIGHT, { -70.0f, offsetY })
                      .Sprite("703x145", spriteName)
                      .Scale({ 0.6f, 0.6f })
                      .BuildSprite();
            if (!btnSprite) return nullptr;

            if (UiButton* btn = b.BuildButton(btnSprite))
            {
                if (zoneIndex < ZONE_BUTTONS)
                {
                    mp_ZoneButtons[zoneIndex] = btn;

                }

                btn->OnClick.ConnectLambda([this, difficulty, zoneIndex]()
                {
                    SelectZone(difficulty);
                });

                UiAnim::On(*this, btnSprite)
                 .FadeTo(1.0f, 2.5f, Tweening::EasingType::Linear)
                 .OnUpdate([btnSprite, btn, zoneIndex, spriteName](float t) {
                     if (btn->IsHovered && !btn->IsDisabled)
                     {
                         int frameNum = static_cast<int>(t * 15.0f);
                         if (frameNum >= 15) frameNum = 0;
                         std::string frameName = "Exp_Zone_" + std::to_string(zoneIndex + 1) + "_ANIM";
                         if (frameNum < 10) frameName += "0";
                         frameName += std::to_string(frameNum);
                         std::string atlasName = "ANIMATION_Exp_Zone_" + std::to_string(zoneIndex + 1);

                         btnSprite->SetSprite(atlasName, frameName);
                     }
                     else
                     {
                        btnSprite->SetSprite("703x145", spriteName);
                     }
                 })
                 .Repeat(-1)
                 .Play();
            }
        }

        if (zoneIndex < ZONE_BUTTONS)
        {
            mp_ZoneSprites[zoneIndex] = btnSprite;
        }

        if (zoneIndex != 0)
        {
            b.ChildOf(btnSprite->GetEntityID())
             .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 5.0f })
             .FontSize(35.0f)
             .Tint(Colors::PERFECT_BROWN)
             .Text(label)
             .BuildText();
        }
        else
        {
            SerialTree tree;
            if (SerialFile::Load(Saves::SAVE_FILE_PATH, tree) != ErrorType::SUCCESS)
            {
                mp_EndlessButtonText = b.ChildOf(btnSprite->GetEntityID())
                                        .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 5.0f })
                                        .FontSize(35.0f)
                                        .Tint(Colors::PERFECT_BROWN)
                                        .Text("")
                                        .BuildText();
            }
            else
            {
                mp_EndlessButtonText = b.ChildOf(btnSprite->GetEntityID())
                                        .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 5.0f })
                                        .FontSize(35.0f)
                                        .Tint(Colors::PERFECT_BROWN)
                                        .Text(label)
                                        .BuildText();
            }
        }


        return btnSprite;
    }

     void SceneExpeditionPrep::RefreshZoneSelection()
     {
         const uint32_t mapping[ZONE_BUTTONS] = { 99, 1, 2, 3 };
         const Color    selected              = Color(170, 210, 170, 255);
         const Color    idle                  = Color(165, 165, 165, 220);
         const Color    tutorialHighlight     = Color(255, 255, 200, 255);

         for (uint32_t i = 0; i < ZONE_BUTTONS; ++i)
         {
             if (mp_ZoneSprites[i] == nullptr) continue;

             const bool isTutorialFocus = m_IsInTutorial && (i == 1);
             bool isActive = isTutorialFocus || (m_SelectedDifficulty == mapping[i]);

              if (mp_ZoneButtons[i])
              {
                  if (m_IsInTutorial)
                  {
                      const bool isZone1 = (i == 1);
                      mp_ZoneButtons[i]->SetDisabled(!isZone1);
                      mp_BackBtn->SetDisabled(true);
                      mp_BackBtn2->SetDisabled(true);
                  }
                  else
                  {
                      mp_ZoneButtons[i]->SetDisabled(false);
                      mp_BackBtn->SetDisabled(false);
                      mp_BackBtn2->SetDisabled(false);
                  }
              }

              isActive = (m_SelectedDifficulty == mapping[i]);
              mp_ZoneSprites[i]->SetTint(isActive ? selected : Colors::WHITE);
              mp_ZoneSprites[i]->DrawOrder = 0;
         }
     }

    void SceneExpeditionPrep::SetupPreparationUI(EntityID parent)
    {
        UiBuilder b(this);

        mp_PreparationUI = b.ChildOf(parent)
                            .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                            .Size({ 1080.0f, 720.0f })
                            .Tint(Color(235, 228, 214, 255))
                            .FitToScreen(true)
                            .BuildCanvas();
        if (!mp_PreparationUI) return;

        const EntityID panel = mp_PreparationUI->GetEntityID();

        b.ChildOf(panel)
                .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                .Sprite("Map", "Map_Background_Blur")
                .BuildSprite();

        UiSprite* titleBg = b.ChildOf(panel)
                             .At(Anchors::TOP_MIDDLE, { 0.0f, -13.0f })
                             .Sprite("613x161", "Prep_Title_Bg")
                             .Scale({ 0.45f, 0.45f })
                             .BuildSprite();

        UiAnim::On(*this, titleBg)
                 .FadeTo(1.0f, 2.5f, Tweening::EasingType::Linear)
                 .OnUpdate([titleBg](float t) {
                         int frameNum = static_cast<int>(t * 14.9f);
                         std::string frameName = "ANIM";
                         if (frameNum < 10) frameName += "0";
                         frameName += std::to_string(frameNum) + "_Prep_Title_Bg";

                         titleBg->SetSprite("ANIMATION_Prep", frameName);
                 })
                 .Repeat(-1)
                 .Play();

        if (titleBg)
        {
            b.ChildOf(titleBg->GetEntityID())
             .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
             .FontSize(26.0f)
             .Tint(Colors::PERFECT_BROWN)
             .Text("PREPARATION")
             .BuildText();
        }

        UiSprite* backBtn = b.ChildOf(panel)
                             .At(Anchors::TOP_LEFT, { 20.0f, 20.0f })
                             .Sprite("218x169", "Button_Delete")
                             .Scale({ 0.4f, 0.4f })
                             .BuildSprite();

        if (backBtn)
        {
            if (mp_BackBtn2 = b.BuildButton(backBtn))
            {
                mp_BackBtn2->OnClick.ConnectLambda([this]()
                {
                    SwitchPage(PrepPage::ZoneSelect);
                });
            }
        }

         mp_SilhouetteAvatar = b.ChildOf(panel)
                                .At(Anchors::MIDDLE_LEFT, { 200.0f, -50.0f })
                                .Sprite("305x652", "Prep_Avatar_Cali")
                                .Scale({ 0.7f, 0.7f })
                                .BuildSprite();
        if (!mp_SilhouetteAvatar) return;

        const EntityID silhouetteId = mp_SilhouetteAvatar->GetEntityID();

        BuildSilhouetteSlot(silhouetteId, SLOT_HELMET,     "Prep_Head", { 0.0f,    -160.0f });
        BuildSilhouetteSlot(silhouetteId, SLOT_CHESTPLATE, "Prep_Body", { 0.0f,     0.0f });
        BuildSilhouetteSlot(silhouetteId, SLOT_GLOVES,     "Prep_Left", { 150.0f,  0.0f });
        BuildSilhouetteSlot(silhouetteId, SLOT_MELEE,      "Prep_Right", { -150.0f,   0.0f });
        BuildSilhouetteSlot(silhouetteId, SLOT_BOOTS,      "Prep_Feet", { 0.0f,     170.0f });

        mp_MeleeAxeOverlay = b.ChildOf(silhouetteId)
                              .At(Anchors::MIDDLE_MIDDLE, { -150.0f, 0.0f })
                              .Sprite("260x227", "Prep_Axe")
                              .Scale({ 0.3f, 0.3f })
                              .BuildSprite();
        if (mp_MeleeAxeOverlay) mp_MeleeAxeOverlay->IsEnable = false;

        mp_HelmetOverlay = b.ChildOf(silhouetteId)
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, -160.0f })
            .Sprite("260x227", "Prep_Helmet")
            .Scale({ 0.3f, 0.3f })
            .BuildSprite();
        if (mp_HelmetOverlay) mp_HelmetOverlay->IsEnable = false;

        mp_ChestPlateOverlay = b.ChildOf(silhouetteId)
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
            .Sprite("260x227", "Prep_ChestPlate")
            .Scale({ 0.3f, 0.3f })
            .BuildSprite();
        if (mp_ChestPlateOverlay) mp_ChestPlateOverlay->IsEnable = false;

        mp_BottomOverlay = b.ChildOf(silhouetteId)
            .At(Anchors::MIDDLE_MIDDLE, { 150.0f, 0.0f })
            .Sprite("260x227", "Prep_Bottom")
            .Scale({ 0.3f, 0.3f })
            .BuildSprite();
        if (mp_BottomOverlay) mp_BottomOverlay->IsEnable = false;

        mp_BootsOverlay = b.ChildOf(silhouetteId)
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 170.0f })
            .Sprite("260x227", "Prep_Boots")
            .Scale({ 0.3f, 0.3f })
            .BuildSprite();
        if (mp_BootsOverlay) mp_BootsOverlay->IsEnable = false;

        BuildHeroPortrait(silhouetteId, ActiveHero::Cali, -80.0f);
        BuildHeroPortrait(silhouetteId, ActiveHero::Iris,  80.0f);

        BuildEquipmentGrid(panel);

        UiSprite* launch = b.ChildOf(panel)
                            .At(Anchors::BOTTOM_RIGHT, { -30.0f, -30.0f })
                            .Sprite("260x227", "Prep_Expedition")
                            .Scale({ 0.55f, 0.55f })
                            .BuildSprite();

        if (launch)
        {
            if (mp_LaunchBtn = b.BuildButton(launch))
            {
                mp_LaunchBtn->OnClick.ConnectLambda([this]()
                {
                    OnLaunchExpedition();
                });
            }
        }
    }

    void SceneExpeditionPrep::BuildHeroPortrait(EntityID parent, ActiveHero hero, float offsetX)
    {
        UiBuilder b(this);

        const std::string label      = (hero == ActiveHero::Cali) ? "Cali" : "Iris";
        const std::string spriteName = (hero == ActiveHero::Cali) ? "Prep_Frame_1" : "Prep_Frame_2";

        UiSprite* portrait = b.ChildOf(parent)
                              .At(Anchors::BOTTOM_MIDDLE, { offsetX, 145.0f })
                              .Sprite("260x227", spriteName)
                              .Scale({ 0.6f, 0.6f })
                              .BuildSprite();
        if (!portrait) return;

        if (hero == ActiveHero::Cali) mp_CaliPortrait = portrait;
        else                          mp_IrisPortrait = portrait;

        if (UiButton* btn = b.BuildButton(portrait))
        {
            btn->OnClick.ConnectLambda([this, hero]()
            {
                SelectHero(hero);
            });
        }
    }

    void SceneExpeditionPrep::BuildSilhouetteSlot(EntityID parent, SilhouetteSlot slot, const std::string& spriteIdle, glm::vec2 position)
    {
        UiBuilder b(this);

        UiSprite* slotSprite = b.ChildOf(parent)
                                .At(Anchors::MIDDLE_MIDDLE, position)
                                .Sprite("260x227", spriteIdle)
                                .Scale({ 0.65f, 0.65f })
                                .BuildSprite();
        if (!slotSprite) return;

        mp_SilhouetteSprites[slot] = slotSprite;
    }

    void SceneExpeditionPrep::BuildEquipmentGrid(EntityID parent)
    {
        UiBuilder b(this);

        UiCanvas* grid = b.ChildOf(parent)
                          .At(Anchors::MIDDLE_MIDDLE, { 200.0f, 20.0f })
                          .Size({ 480.0f, 560.0f })
                          .Tint(Colors::TRANSPARENT)
                          .BuildCanvas();
        if (!grid) return;

        static const char* kSprites[GRID_SLOTS] = {
            "Prep_Inv_Axe",
            "Prep_Inv_Helmet",
            "Prep_Inv_2",
            "Prep_Inv_ChestPlate",
            "Prep_Inv_4",
            "Prep_Inv_Bottom",
            "Prep_Inv_6",
            "Prep_Inv_Boots"
        };

        for (uint32_t i = 0; i < GRID_SLOTS; ++i)
        {
            BuildGridSlot(grid->GetEntityID(), i, kSprites[i]);
        }

        BuildPrepTooltip(grid->GetEntityID());
    }

    void SceneExpeditionPrep::BuildGridSlot(EntityID parent, uint32_t index, const std::string& sprite)
    {
        UiBuilder b(this);

        constexpr uint32_t COLS         = 2;
        constexpr float SPRITE_SCALE    = 0.6f;
        constexpr float SLOT_W          = 400.0f * SPRITE_SCALE;
        constexpr float SLOT_H          = 50.0f * SPRITE_SCALE;
        constexpr float GAP_X           = 15.0f;
        constexpr float GAP_Y           = 75.0f;

        const uint32_t col = index % COLS;
        const uint32_t row = index / COLS;
        const float    x   = 5.0f + col * (SLOT_W + GAP_X);
        const float    y   = 20.0f + row * (SLOT_H + GAP_Y);

        bool isUnlocked = (index == 0)
                       || (index == 1)
                       || (index == 3)
                       || (index == 5)
                       || (index == 7);

        UiSprite* slot = b.ChildOf(parent)
                          .At(Anchors::TOP_LEFT, { x, y })
                          .Sprite("613x161", sprite)
                          .Scale({ SPRITE_SCALE, SPRITE_SCALE })
                          .BuildSprite();

        if (!slot) return;

        mp_GridSprites[index] = slot;

        if (UiButton* btn = b.BuildButton(slot))
        {
            mp_GridButtons[index] = btn;
            btn->IsDisabled = !isUnlocked;

            btn->OnClick.ConnectLambda([this, index, isUnlocked]()
            {
                if (isUnlocked)
                {
                    EquipSlotItem(index);
                }
            });

            if (isUnlocked)
            {
                btn->OnHover.ConnectLambda([this, index]()
                {
                    ShowTooltipForSlot(index);
                });
                btn->OnLeave.ConnectLambda([this]()
                {
                    HideTooltip();
                });
            }
        }

        std::string label = "---";
        if (isUnlocked)
        {
            const std::string tier = " T1";
            switch (index)
            {
                case 0: label = "";                  break;
                case 1: label = "Helmet"     + tier; break;
                case 2: label = "ChestPlate" + tier; break;
                case 3: label = "Gloves"     + tier; break;
                case 4: label = "Legs"       + tier; break;
                case 5: label = "Boots"      + tier; break;
                case 6: label = "LongSword"  + tier; break;
                case 7: label = "---";               break;
                default: label = "Item";             break;
            }
        }
    }

    void SceneExpeditionPrep::SetupSettingsUI(EntityID parent)
    {
        UiBuilder b(this);

        UiSprite* toggle = b.ChildOf(parent)
                            .At(Anchors::TOP_RIGHT, { 40.0f, -30.0f })
                            .Scale({0.8f,0.8f})
                            .Sprite("218x169", "Button_Settings")
                            .BuildSprite();

        UiButton* button = b.BuildButton(toggle);
        if (!button) return;
        button->OnClick.ConnectLambda([this]()
        {
            settingsIsVisible = !settingsIsVisible;
            if (mp_SettingsBehaviour) mp_SettingsBehaviour->Toggle(settingsIsVisible);
        });

        if (mp_SettingsBehaviour)
        {
            mp_SettingsBehaviour->OnSettingsChanged.ConnectLambda([]() {
                SettingsGame::Get().Save();
            });

            mp_SettingsBehaviour->OnCloseRequested.ConnectLambda([this]()
            {
                settingsIsVisible = !settingsIsVisible;
                // TODO : mp_UIpopClose->Play();
            });
        }
    }

    void SceneExpeditionPrep::SwitchPage(PrepPage page)
    {
        m_Page = page;
        if (mp_ZoneSelectUI)  SetHierarchyEnabled(mp_ZoneSelectUI->GetEntityID(),  page == PrepPage::ZoneSelect);
        if (mp_PreparationUI) SetHierarchyEnabled(mp_PreparationUI->GetEntityID(), page == PrepPage::Preparation);

        if (page == PrepPage::Preparation)
        {
            RefreshSilhouette();
            RefreshGridVisual();
            HideTooltip();
        }
    }

    void SceneExpeditionPrep::SelectZone(uint32_t difficulty)
    {
        m_SelectedDifficulty = difficulty;
        RefreshZoneSelection();
        SwitchPage(PrepPage::Preparation);
    }

    void SceneExpeditionPrep::SelectHero(ActiveHero hero)
    {
        m_ActiveHero = hero;
        RebuildGridForActiveHero();
        RefreshActiveHeroLabel();
        RefreshSilhouette();
        RefreshGridVisual();
        RefreshHeroSelection();
    }

    void SceneExpeditionPrep::EquipSlotItem(uint32_t slotIndex)
    {
        if (slotIndex >= m_GridItems.size()) return;
        Equipment* pItem = m_GridItems[slotIndex];
        if (pItem == nullptr) return;

        if (slotIndex == 0 || slotIndex == 6)
        {
            const OwnerCharacter expected = ActiveHeroToOwner(m_ActiveHero);
            if (pItem->Owner != expected) return;
        }

        HeroLoadout& loadout = ActiveLoadout();

        switch (slotIndex)
        {
            case 0:
            case 6: loadout.pMelee      = (loadout.pMelee      == pItem) ? nullptr : static_cast<MeleeWeapon*>(pItem); break;
            case 1: loadout.pHelmet     = (loadout.pHelmet     == pItem) ? nullptr : static_cast<Helmet*>(pItem);      break;
            case 3: loadout.pChestPlate = (loadout.pChestPlate == pItem) ? nullptr : static_cast<ChestPlate*>(pItem);  break;
            case 5: loadout.pGloves     = (loadout.pGloves     == pItem) ? nullptr : static_cast<Gloves*>(pItem);      break;
            case 7: loadout.pBoots      = (loadout.pBoots      == pItem) ? nullptr : static_cast<Boots*>(pItem);       break;
            default: break;
        }

        RefreshSilhouette();
        RefreshGridVisual();
    }

    void SceneExpeditionPrep::RefreshSilhouette()
    {
        if (mp_SilhouetteAvatar)
        {
            std::string avatarSprite = (m_ActiveHero == ActiveHero::Cali) ? "Prep_Avatar_Cali" : "Prep_Avatar_Iris";
            mp_SilhouetteAvatar->SetSprite("305x652", avatarSprite);
        }

        const HeroLoadout& loadout = ActiveLoadout();

        auto swap = [&](SilhouetteSlot slot, bool equipped, const std::string& idle, const std::string& sel)
        {
            if (mp_SilhouetteSprites[slot] == nullptr) return;

            if (slot == SLOT_MELEE)
            {
                mp_SilhouetteSprites[slot]->SetSprite("260x227", equipped ? sel : idle);
            }
            else if (slot == SLOT_HELMET)
            {
                mp_SilhouetteSprites[slot]->SetSprite("260x227", equipped ? sel : idle);
            }
            else if (slot == SLOT_CHESTPLATE)
            {
                mp_SilhouetteSprites[slot]->SetSprite("260x227", equipped ? sel : idle);
            }
            else if (slot == SLOT_GLOVES)
            {
                mp_SilhouetteSprites[slot]->SetSprite("260x227", equipped ? sel : idle);
            }
            else if (slot == SLOT_BOOTS)
            {
                mp_SilhouetteSprites[slot]->SetSprite("260x227", equipped ? sel : idle);
            }
            else
            {
                mp_SilhouetteSprites[slot]->SetSprite("260x227", idle);
            }
        };

        swap(SLOT_HELMET,     loadout.pHelmet     != nullptr, "Prep_Head",  "Prep_Head_Sel");
        swap(SLOT_CHESTPLATE, loadout.pChestPlate != nullptr, "Prep_Body",  "Prep_Body_Sel");
        swap(SLOT_GLOVES,     loadout.pGloves     != nullptr, "Prep_Left",  "Prep_Left_Sel");

        swap(SLOT_MELEE,      loadout.pMelee      != nullptr, "Prep_Right", "Prep_Right_Sel");

        if (mp_MeleeAxeOverlay != nullptr)
        {
            const std::string overlaySprite = (m_ActiveHero == ActiveHero::Cali) ? "Prep_CaliWeapon" : "Prep_Axe";
            mp_MeleeAxeOverlay->SetSprite("260x227", overlaySprite);
            mp_MeleeAxeOverlay->IsEnable = (loadout.pMelee != nullptr);
        }

        swap(SLOT_BOOTS,      loadout.pBoots      != nullptr, "Prep_Feet",  "Prep_Feet_Sel");

        if (mp_HelmetOverlay)     mp_HelmetOverlay->IsEnable     = (loadout.pHelmet     != nullptr);
        if (mp_ChestPlateOverlay) mp_ChestPlateOverlay->IsEnable = (loadout.pChestPlate != nullptr);
        if (mp_BottomOverlay)     mp_BottomOverlay->IsEnable     = (loadout.pGloves     != nullptr);
        if (mp_BootsOverlay)      mp_BootsOverlay->IsEnable      = (loadout.pBoots      != nullptr);
    }

    void SceneExpeditionPrep::RefreshGridVisual()
    {
        const HeroLoadout& loadout = ActiveLoadout();

        for (uint32_t i = 0; i < GRID_SLOTS; ++i)
        {
            if (mp_GridSprites[i] == nullptr) continue;

            switch (i)
            {
            case 0:
                {
                    bool eq = (loadout.pMelee != nullptr && loadout.pMelee == m_GridItems[0]);
                    const bool isCali = (m_ActiveHero == ActiveHero::Cali);
                    const std::string idle = isCali ? "Prep_Inv_Gauntlet"     : "Prep_Inv_Axe";
                    const std::string sel  = isCali ? "Prep_Inv_Gauntlet_Sel" : "Prep_Inv_Axe_Sel";
                    mp_GridSprites[0]->SetSprite("613x161", eq ? sel : idle);
                    break;
                }
            case 1:
                {
                    bool eq = (loadout.pHelmet != nullptr);
                    mp_GridSprites[1]->SetSprite("613x161", eq ? "Prep_Inv_Helmet_Sel" : "Prep_Inv_Helmet");
                    break;
                }
            case 3:
                {
                    bool eq = (loadout.pChestPlate != nullptr);
                    mp_GridSprites[3]->SetSprite("613x161", eq ? "Prep_Inv_ChestPlate_Sel" : "Prep_Inv_ChestPlate");
                    break;
                }
            case 5:
                {
                    bool eq = (loadout.pGloves != nullptr);
                    mp_GridSprites[5]->SetSprite("613x161", eq ? "Prep_Inv_Bottom_Sel" : "Prep_Inv_Bottom");
                    break;
                }
            case 7:
                {
                    bool eq = (loadout.pBoots != nullptr);
                    mp_GridSprites[7]->SetSprite("613x161", eq ? "Prep_Inv_Boots_Sel" : "Prep_Inv_Boots");
                    break;
                }
            default: break;
            }
        }
    }

    void SceneExpeditionPrep::RefreshHeroSelection()
    {
        const Color selected = Colors::WHITE;
        const Color dimmed   = Color(100, 100, 100, 255);

        if (mp_CaliPortrait) mp_CaliPortrait->SetTint(m_ActiveHero == ActiveHero::Cali ? selected : dimmed);
        if (mp_IrisPortrait) mp_IrisPortrait->SetTint(m_ActiveHero == ActiveHero::Iris ? selected : dimmed);
    }

    void SceneExpeditionPrep::RefreshActiveHeroLabel()
    {
        if (mp_ActiveHeroLabel == nullptr) return;
        const std::string name = (m_ActiveHero == ActiveHero::Cali) ? "Cali" : "Iris";
        mp_ActiveHeroLabel->SetText(name);
    }

    void SceneExpeditionPrep::OnLaunchExpedition()
    {
        if (m_LaunchInProgress) return;

        SceneExpeditionStartCtx* pPending = new SceneExpeditionStartCtx();
        pPending->MissionDifficulty = m_SelectedDifficulty;
        pPending->pHelmet       = m_CaliLoadout.pHelmet;
        pPending->pGloves       = m_CaliLoadout.pGloves;
        pPending->pChestPlate   = m_CaliLoadout.pChestPlate;
        pPending->pLegs         = m_CaliLoadout.pLegs;
        pPending->pBoots        = m_CaliLoadout.pBoots;
        pPending->pMelee        = m_CaliLoadout.pMelee;
        pPending->pDistance     = nullptr;

        SceneVillageStartCtx* pVillage = new SceneVillageStartCtx();
        pVillage->pPendingExpedition = pPending;

        if (!mp_LaunchingScreen)
        {
            SceneChangeCtx ctx{};
            ctx.pData    = pVillage;
            ctx.DataSize = sizeof(SceneVillageStartCtx);
            SwapToScene("Village", ctx);
            return;
        }

        m_LaunchInProgress = true;
        SetHierarchyEnabled(mp_LaunchingScreen->GetEntityID(), true);
        if (mp_LaunchingBackground) mp_LaunchingBackground->SetTint(Color(0, 0, 0, 0));
        if (mp_LaunchingText)       mp_LaunchingText->SetTint(Color(255, 255, 255, 0));

        Scene* pScene = this;
        SceneVillageStartCtx* pCapturedCtx = pVillage;

        const auto applyAlpha = [this](uint8_t a)
        {
            if (mp_LaunchingBackground) mp_LaunchingBackground->SetTint(Color(0, 0, 0, a));
            if (mp_LaunchingText)       mp_LaunchingText->SetTint(Color(255, 255, 255, a));
        };

        constexpr float kFadeInDuration = 0.5f;

        UiAnimSlot fadeIn{};
        fadeIn.Delay    = 0.0f;
        fadeIn.Duration = kFadeInDuration;
        fadeIn.Ease     = Tweening::EasingType::EaseInQuad;
        fadeIn.Setter = [applyAlpha](float t)
        {
            applyAlpha(static_cast<uint8_t>(255.0f * t));
        };
        fadeIn.OnComplete = [pScene, pCapturedCtx]()
        {
            SceneChangeCtx ctx{};
            ctx.pData    = pCapturedCtx;
            ctx.DataSize = sizeof(SceneVillageStartCtx);
            pScene->SwapToScene("Village", ctx);
        };
        UiAnim::PushAnim(*this, mp_LaunchingScreen->GetEntityID(), std::move(fadeIn));
    }

    void SceneExpeditionPrep::SetupLaunchingScreenUI()
    {
        UiBuilder b(this);

        mp_LaunchingScreen = b
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
            .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
            .Tint(Colors::TRANSPARENT)
            .FitToScreen(true)
            .BuildCanvas();
        if (!mp_LaunchingScreen) return;

        mp_LaunchingBackground = b.ChildOf(mp_LaunchingScreen->GetEntityID())
            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
            .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
            .Scale({2.0f, 2.0f})
            .Tint(Color(0, 0, 0, 0))
            .BuildSprite();

        SetHierarchyEnabled(mp_LaunchingScreen->GetEntityID(), false);
    }

    void SceneExpeditionPrep::OnBackToVillage()
    {
        SceneChangeCtx ctx{};
        ctx.pData    = nullptr;
        ctx.DataSize = 0;
        SwapToScene("Village", ctx);
    }

    SceneExpeditionPrep::HeroLoadout& SceneExpeditionPrep::ActiveLoadout()
    {
        return (m_ActiveHero == ActiveHero::Cali) ? m_CaliLoadout : m_IrisLoadout;
    }

    void SceneExpeditionPrep::EnsureGridItems()
    {
        EnsurePoolItems();
        RebuildGridForActiveHero();
    }

    void SceneExpeditionPrep::EnsurePoolItems()
    {
        if (mp_VillageInventory == nullptr) return;

        bool hasCaliWeapon = false;
        bool hasIrisWeapon = false;
        bool hasHelmet     = false;
        bool hasChest      = false;
        bool hasGloves     = false;
        bool hasBoots      = false;

        for (EquipmentID id = 0; id < MAX_EQUIPMENTS; ++id)
        {
            if (!mp_VillageInventory->HasEquipment(id)) continue;
            Equipment* pEq = mp_VillageInventory->GetEquipment(id);
            if (!pEq) continue;

            if (pEq->Owner == OwnerCharacter::Cali && dynamic_cast<MeleeWeapon*>(pEq)) hasCaliWeapon = true;
            if (pEq->Owner == OwnerCharacter::Iris && dynamic_cast<MeleeWeapon*>(pEq)) hasIrisWeapon = true;
            if (dynamic_cast<Helmet*>(pEq))     hasHelmet = true;
            if (dynamic_cast<ChestPlate*>(pEq)) hasChest  = true;
            if (dynamic_cast<Gloves*>(pEq))     hasGloves = true;
            if (dynamic_cast<Boots*>(pEq))      hasBoots  = true;
        }

        auto pushNew = [&](Equipment* pEq)
        {
            pEq->OnBaseStatistics();
            pEq->ReRollTraits();
            mp_VillageInventory->AddEquipment(pEq);
        };

        if (!hasCaliWeapon) pushNew(new Gauntlet(1));
        if (!hasIrisWeapon) pushNew(new Axe(1));
        if (!hasHelmet)     pushNew(new Helmet(1));
        if (!hasChest)      pushNew(new ChestPlate(1));
        if (!hasGloves)     pushNew(new Gloves(1));
        if (!hasBoots)      pushNew(new Boots(1));
    }

    void SceneExpeditionPrep::RebuildGridForActiveHero()
    {
        m_GridItems.assign(GRID_SLOTS, nullptr);
        if (mp_VillageInventory == nullptr) return;

        const OwnerCharacter activeOwner = ActiveHeroToOwner(m_ActiveHero);

        for (EquipmentID id = 0; id < MAX_EQUIPMENTS; ++id)
        {
            if (!mp_VillageInventory->HasEquipment(id)) continue;
            Equipment* pEq = mp_VillageInventory->GetEquipment(id);
            if (!pEq) continue;

            if (dynamic_cast<MeleeWeapon*>(pEq) && pEq->Owner == activeOwner)
            {
                if (m_GridItems[0] == nullptr) m_GridItems[0] = pEq;
                continue;
            }

            if (pEq->Owner != OwnerCharacter::Shared) continue;

            if      (dynamic_cast<Helmet*>(pEq)     && m_GridItems[1] == nullptr) m_GridItems[1] = pEq;
            else if (dynamic_cast<ChestPlate*>(pEq) && m_GridItems[3] == nullptr) m_GridItems[3] = pEq;
            else if (dynamic_cast<Gloves*>(pEq)     && m_GridItems[5] == nullptr) m_GridItems[5] = pEq;
            else if (dynamic_cast<Boots*>(pEq)      && m_GridItems[7] == nullptr) m_GridItems[7] = pEq;
        }
    }

    OwnerCharacter SceneExpeditionPrep::ActiveHeroToOwner(ActiveHero hero)
    {
        return (hero == ActiveHero::Cali) ? OwnerCharacter::Cali : OwnerCharacter::Iris;
    }

    Equipment* SceneExpeditionPrep::FindFirstByOwner(OwnerCharacter owner)
    {
        if (mp_VillageInventory == nullptr) return nullptr;
        for (EquipmentID id = 0; id < MAX_EQUIPMENTS; ++id)
        {
            if (!mp_VillageInventory->HasEquipment(id)) continue;
            Equipment* pEq = mp_VillageInventory->GetEquipment(id);
            if (pEq && pEq->Owner == owner) return pEq;
        }
        return nullptr;
    }

    void SceneExpeditionPrep::ResetLoadouts()
    {
        m_CaliLoadout = {};
        m_IrisLoadout = {};
    }

    void SceneExpeditionPrep::SetHierarchyEnabled(EntityID root, bool enabled)
    {
        if (!m_UIBuilt) return;
        if (!HasComponent<Transform2D>(root)) return;

        if (HasComponent<UiCanvas>(root))
            GetComponent<UiCanvas>(root)->IsEnable = enabled;
        if (HasComponent<UiSprite>(root))
            GetComponent<UiSprite>(root)->IsEnable = enabled;
        if (HasComponent<UiText>(root))
            GetComponent<UiText>(root)->IsEnable = enabled;

        Transform2D* pTransform = GetComponent<Transform2D>(root);
        if (pTransform == nullptr) return;

        for (uint32_t i = 0; i < pTransform->GetChildCount(); ++i)
        {
            Transform2D* pChild = pTransform->GetChild(i);
            if (pChild == nullptr) continue;
            SetHierarchyEnabled(pChild->GetEntityID(), enabled);
        }
    }

    void SceneExpeditionPrep::BuildPrepTooltip(EntityID parent)
    {
        UiBuilder b(this);

        constexpr float TOOLTIP_SCALE = 0.22f;
        constexpr float TOOLTIP_W     = 1443.0f * TOOLTIP_SCALE;
        constexpr float TOOLTIP_H     = 998.0f * TOOLTIP_SCALE;

        mp_PrepTooltipCanvas = b.ChildOf(parent)
                                .At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                                .Size({ TOOLTIP_W, TOOLTIP_H })
                                .Tint(Colors::TRANSPARENT)
                                .BuildCanvas();
        if (!mp_PrepTooltipCanvas) return;

        mp_PrepTooltipCanvas->IsEnable = false;

        mp_PrepTooltipBg = b.ChildOf(mp_PrepTooltipCanvas->GetEntityID())
                            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
                            .Sprite("1443x998", "Settings_Background")
                            .Scale({ TOOLTIP_SCALE, TOOLTIP_SCALE })
                            .BuildSprite();

        mp_TooltipTitleLabel = b.ChildOf(mp_PrepTooltipCanvas->GetEntityID())
                                .At(Anchors::TOP_MIDDLE, { 0.0f, 35.0f })
                                .FontSize(18.0f)
                                .Tint(Colors::PERFECT_BROWN)
                                .Text("")
                                .BuildText();
        if (mp_TooltipTitleLabel) mp_TooltipTitleLabel->BindReactive(m_TooltipTitle);

        constexpr float STAT_START_Y = 70.0f;
        constexpr float STAT_LINE_H  = 18.0f;
        for (uint32_t i = 0; i < TOOLTIP_STAT_LINES; ++i)
        {
            UiText* line = b.ChildOf(mp_PrepTooltipCanvas->GetEntityID())
                            .At(Anchors::TOP_MIDDLE, { 0.0f, STAT_START_Y + i * STAT_LINE_H })
                            .FontSize(13.0f)
                            .Tint(Colors::PERFECT_BROWN)
                            .Text("")
                            .BuildText();
            mp_TooltipStatLabels[i] = line;
            if (line) line->BindReactive(m_TooltipStatLines[i]);
        }

        mp_TooltipForgeHint = b.ChildOf(mp_PrepTooltipCanvas->GetEntityID())
                               .At(Anchors::BOTTOM_MIDDLE, { 0.0f, -30.0f })
                               .FontSize(11.0f)
                               .Tint(Colors::PERFECT_BROWN)
                               .Text("Passe par la forge pour reroll les stats !")
                               .BuildText();
    }

    void SceneExpeditionPrep::ShowTooltipForSlot(uint32_t slotIndex)
    {
        if (mp_PrepTooltipCanvas == nullptr) return;
        if (slotIndex >= m_GridItems.size())
        {
            HideTooltip();
            return;
        }

        Equipment* pEq = m_GridItems[slotIndex];
        if (pEq == nullptr)
        {
            HideTooltip();
            return;
        }

        if (pEq->GetBaseStatistics().empty())
        {
            pEq->OnBaseStatistics();
        }

        m_TooltipTitle.Set(EquipmentDisplayName(pEq));

        const auto& baseStats = pEq->GetBaseStatistics();
        for (uint32_t i = 0; i < TOOLTIP_STAT_LINES; ++i)
        {
            if (i < baseStats.size())
            {
                m_TooltipStatLines[i].Set(FormatStat(baseStats[i]));
            }
            else
            {
                m_TooltipStatLines[i].Set("");
            }
        }

        constexpr float TOOLTIP_W = 1443.0f * 0.22f;
        constexpr float MARGIN_X  = 10.0f;

        UiSprite* pSlotSprite = mp_GridSprites[slotIndex];
        if (pSlotSprite != nullptr)
        {
            Transform2D* pSlotT = GetComponent<Transform2D>(pSlotSprite->GetEntityID());
            if (pSlotT != nullptr)
            {
                glm::vec3 slotPos = pSlotT->LocalTransform.GetPosition();
                float targetX = slotPos.x - TOOLTIP_W - MARGIN_X;
                float targetY = slotPos.y;
                UiAnim::MoveTo(*this, mp_PrepTooltipCanvas, { targetX, targetY }, 0.0f, Tweening::EasingType::Linear);
            }
        }

        mp_PrepTooltipCanvas->IsEnable = true;
    }

    void SceneExpeditionPrep::HideTooltip()
    {
        if (mp_PrepTooltipCanvas != nullptr)
        {
            mp_PrepTooltipCanvas->IsEnable = false;
        }
    }

    std::string SceneExpeditionPrep::EquipmentDisplayName(Equipment* pEq)
    {
        if (pEq == nullptr) return "";
        if (dynamic_cast<Gauntlet*>(pEq))   return "Gantelet";
        if (dynamic_cast<Axe*>(pEq))        return "Hache";
        if (dynamic_cast<ShortSword*>(pEq)) return "Epee Courte";
        if (dynamic_cast<LongSword*>(pEq))  return "Epee Longue";
        if (dynamic_cast<Tomahawk*>(pEq))   return "Tomahawk";
        if (dynamic_cast<Bow*>(pEq))        return "Arc";
        if (dynamic_cast<Helmet*>(pEq))     return "Casque";
        if (dynamic_cast<ChestPlate*>(pEq)) return "Plastron";
        if (dynamic_cast<Gloves*>(pEq))     return "Gants";
        if (dynamic_cast<Legs*>(pEq))       return "Jambieres";
        if (dynamic_cast<Boots*>(pEq))      return "Bottes";
        return "Equipement";
    }

    std::string SceneExpeditionPrep::FormatStat(const BaseStat& stat)
    {
        std::string name = (stat.Name != nullptr) ? stat.Name : "";
        std::string valStr = std::to_string(stat.Value);
        valStr.erase(valStr.find_last_not_of('0') + 1, std::string::npos);
        if (!valStr.empty() && valStr.back() == '.') valStr.pop_back();
        return name + " : " + valStr;
    }
}
