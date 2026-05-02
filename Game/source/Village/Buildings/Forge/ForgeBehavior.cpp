#include "Village/Buildings/Forge/ForgeBehavior.h"
#include "ParticleSystem.h"
#include "UI/UiBuilder.h"
#include "UI/UiCanvas.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"
#include "UI/UiButton.h"
#include "UI/UiTheme.h"
#include "Scene.h"
#include "Expedition/ResourceBehavior.h"
#include "Scenes/SceneVillageV1.h"
#include "Village/VillageManager.h"
#include "Village/Inventory.h"
#include "Village/BuildingData.h"
#include "Expedition/Stats.h"
#include "Expedition/Equipment.h"

namespace GPC
{
    namespace ForgeUIConfig
    {
        constexpr glm::vec2     CanvasSize        = { 786.0f, 860.0f };
        constexpr glm::vec2     BgOffset          = { -20.0f, 0.0f };
        constexpr glm::vec2     BgScale           = { 0.6f, 0.6f };
        constexpr glm::vec2     TitleOffset       = { 170.0f, -170.0f };
        constexpr float         TitleFontSize     = 36.0f;
        constexpr float         GridStartX        = -60.0f;
        constexpr float         GridStartY        = -120.0f;
        constexpr float         GridSpacingX      = 200.0f;
        constexpr float         GridSpacingY      = 90.0f;
        constexpr glm::vec2     ButtonScale       = { 0.6f, 0.6f };
        constexpr float         GemStartX         = -60.0f;
        constexpr float         GemStartY         = 160.0f;
        constexpr float         GemSpacingX       = 65.0f;
        constexpr glm::vec2     GemIconScale      = { 1.f, 1.f };
        constexpr glm::vec2     GemTextOffset     = { 10.0f, -10.0f };
        constexpr float         GemTextFontSize   = 18.0f;

        constexpr glm::vec2     DetailPanelOffset = { 350.0f, 0.0f };
        constexpr glm::vec2     DetailPanelSize   = { 400.0f, 700.0f };
        constexpr glm::vec2     DetailBgScale     = { 0.9f, 0.9f };
        constexpr glm::vec2     DetailTitleOffset = { -250.0f, -120.0f };
        constexpr float         DetailTitleSize   = 34.0f;
        constexpr float         StatStartY        = -70.0f;
        constexpr float         StatSpacingY      = 50.0f;
        constexpr glm::vec2     StatNameOffset    = { 90.0f, 0.0f };
        constexpr glm::vec2     StatValueOffset   = { 250.0f, 0.0f };
        constexpr glm::vec2     SocketOffset      = { -200.0f, 0.0f };
        constexpr glm::vec2     SocketScale       = { 0.6f, 0.6f };
        constexpr glm::vec2     UpgradeOffset     = { -100.0f, -220.0f };
        constexpr glm::vec2     UpgradeScale      = { 0.2f, 0.3f };
        constexpr glm::vec2     UpgradeTextOffset = { 0.0f, 0.0f };
        constexpr float         UpgradeTextSize   = 22.0f;
    }

    std::string ForgeBehavior::GetEquipmentName(Equipment* pEq) const
    {
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
        return "None";
    }

    std::string ForgeBehavior::GetEquipmentTexture(Equipment* pEquip) const
    {
        if (dynamic_cast<Gauntlet*>(pEquip)) return "Forge_Hache";
        if (dynamic_cast<Axe*>(pEquip)) return "Forge_Hache";
        if (dynamic_cast<ShortSword*>(pEquip)) return "Forge_Hache";
        if (dynamic_cast<Gloves*>(pEquip)) return "Forge_Button_6";
        return "None";
    }

    void ForgeBehavior::OnCreate(const BehaviorCreateContext* pCtx)
    {
        BuildingBehavior::OnCreate(pCtx);
        if (pCtx && pCtx->pScene)
        {
            mp_Scene = pCtx->pScene;
            auto* villageScene = dynamic_cast<SceneVillageV1*>(pCtx->pScene);
            if (villageScene)
            {
                m_VillageManagerCache = villageScene->GetVillageManager();
            }
        }

        EntityID id = pCtx->pScene->CreateEntityAs3D();
        Transform3D* transform = pCtx->pScene->GetComponent<Transform3D>(id);
        transform->LocalTransform.SetPosition(pCtx->pScene->GetComponent<Transform3D>(GetEntityID())->LocalTransform.GetPosition());
        transform->LocalTransform.AddPosition(1.4f , 4.4f, 0.0f);
        auto emitter = pCtx->pScene->AddComponent<ParticleEmitter>(id);
        emitter->pGeometry = GPC_MESH("Sphere");
        emitter->pComputeProgram = &GPC_COMPUTE_PROGRAM("Forge Particle Program")->Program;
        emitter->SetLooping(true);
        emitter->AutoStart();
    }

    void ForgeBehavior::OnStart(const BehaviorCreateContext *pCtx)
    {
        BuildingBehavior::OnStart(pCtx);
        CreateBaseForgeUI(pCtx);
    }

    void ForgeBehavior::OnUpdate(const BehaviorUpdateContext* pCtx)
    {
        BuildingBehavior::OnUpdate(pCtx);

        if (m_VillageManagerCache)
        {
            const auto& inv = m_VillageManagerCache->GetInventory();
            m_GemCounts[0].Set(static_cast<int>(inv.GetResourceCount<GreenGemme>()));
            m_GemCounts[1].Set(static_cast<int>(inv.GetResourceCount<RedGemme>()));
            m_GemCounts[2].Set(static_cast<int>(inv.GetResourceCount<BlueGemme>()));
            m_GemCounts[3].Set(static_cast<int>(inv.GetResourceCount<OrangeGemme>()));
            m_GemCounts[4].Set(static_cast<int>(inv.GetResourceCount<PurpleGemme>()));
            m_GemCounts[5].Set(static_cast<int>(inv.GetResourceCount<YellowGemme>()));
        }
    }

    void ForgeBehavior::OnInteract()
    {
        BuildingBehavior::OnInteract();
        RefreshInventoryUI();
        if (m_ForgeCanvas) m_ForgeCanvas->IsEnable = true;
        if (m_DetailCanvas) m_DetailCanvas->IsEnable = false;
    }

    void ForgeBehavior::OnNextCycle()
    {
    }

    void ForgeBehavior::CloseOpened()
    {
        BuildingBehavior::CloseOpened();
        if (m_ForgeCanvas) m_ForgeCanvas->IsEnable = false;
        if (m_DetailCanvas) m_DetailCanvas->IsEnable = false;
        m_CurrentEquip = nullptr;
    }

    void ForgeBehavior::CreateBaseForgeUI(const BehaviorCreateContext* pCtx)
    {
        if (!mp_Scene) return;

        UiBuilder b(mp_Scene);
        using namespace ForgeUIConfig;

        m_ForgeCanvas = b.At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
                         .Size(CanvasSize)
                         .BuildCanvas();
        
        if (!m_ForgeCanvas) return;
        EntityID canvasId = m_ForgeCanvas->GetEntityID();

        b.ChildOf(canvasId)
         .At(Anchors::MIDDLE_LEFT, BgOffset)
         .Scale(BgScale)
         .Sprite("786x860", "Background_Forge_Panel")
         .BuildSprite();

        b.ChildOf(canvasId)
         .At(Anchors::MIDDLE_LEFT, TitleOffset)
         .Font(UiTheme::DefaultFont, TitleFontSize)
         .Tint(Colors::PERFECT_BROWN)
         .Text("FORGE")
         .BuildText();

        m_EmptyCanvas = b.ChildOf(canvasId)
                         .At(Anchors::MIDDLE_LEFT, { 0.0f, 0.0f })
                         .BuildCanvas();

        b.ChildOf(m_EmptyCanvas->GetEntityID())
         .At(Anchors::MIDDLE_LEFT, { GridStartX + 120.0f, GridStartY + 60.0f })
         .Font(UiTheme::DefaultFont, 36.0f)
         .Tint(Colors::PERFECT_BROWN)
         .Text("Inventaire vide")
         .BuildText();

        for (int i = 0; i < 6; ++i)
        {
            int row = i / 2; int col = i % 2;
            float posX = GridStartX + (col * GridSpacingX);
            float posY = GridStartY + (row * GridSpacingY);
            m_EquipSprites[i] = b.ChildOf(canvasId)
                                 .At(Anchors::MIDDLE_LEFT, { posX, posY })
                                 .Scale(ButtonScale)
                                 .Sprite("580x157", "Forge_Hache")
                                 .BuildSprite();
            if (m_EquipSprites[i])
            {
                UiButton* btn = b.BuildButton(m_EquipSprites[i]);
                btn->OnClick.ConnectLambda([this, i]() { BuildEquipDetailPanel(i); });
                m_EquipSprites[i]->IsEnable = false;
            }
        }

        m_DetailCanvas = b.At(Anchors::MIDDLE_RIGHT, DetailPanelOffset).Size(DetailPanelSize).BuildCanvas();
        if (m_DetailCanvas)
        {
            EntityID detailId = m_DetailCanvas->GetEntityID();
            b.ChildOf(detailId)
             .At(Anchors::MIDDLE_RIGHT, { 0.0f, 0.0f })
             .Scale(DetailBgScale)
             .Sprite("786x860", "Background_Forge_Panel_Upgrade")
             .BuildSprite();

            m_DetailTitle = b.ChildOf(detailId)
                             .At(Anchors::MIDDLE_RIGHT, DetailTitleOffset)
                             .Font(UiTheme::DefaultFont, DetailTitleSize)
                             .Tint(Colors::PERFECT_BROWN)
                             .Text("Equipement")
                             .BuildText();

            for (int i = 0; i < 6; ++i)
            {
                float posY = StatStartY + (i * StatSpacingY);
                m_StatNamesText[i] = b.ChildOf(detailId)
                                      .At(Anchors::MIDDLE_MIDDLE, { StatNameOffset.x, posY })
                                      .Font(UiTheme::DefaultFont, 28.0f)
                                      .Tint(Colors::BLACK)
                                      .Text("Stat")
                                      .BuildText();

                m_StatValuesText[i] = b.ChildOf(detailId)
                                       .At(Anchors::MIDDLE_MIDDLE, { StatValueOffset.x, posY })
                                       .Font(UiTheme::DefaultFont, 28.0f)
                                       .Tint(Colors::BLACK)
                                       .Text("0.0")
                                       .BuildText();

                m_SocketSprites[i] = b.ChildOf(detailId)
                                      .At(Anchors::MIDDLE_RIGHT, { SocketOffset.x, posY })
                                      .Scale(SocketScale)
                                      .Sprite("218x169", "NoaLeGOOOAT")
                                      .BuildSprite();

                if (m_SocketSprites[i])
                {
                    UiButton* socketBtn = b.BuildButton(m_SocketSprites[i]);
                    socketBtn->OnClick.ConnectLambda([this, i]() { OnSocketClicked(i); });
                }
            }

            UiSprite* upgradeSprite = b.ChildOf(detailId)
                                       .At(Anchors::BOTTOM_RIGHT, UpgradeOffset)
                                       .Scale(UpgradeScale)
                                       .Sprite("580x157", "Button_Upgrade")
                                       .BuildSprite();

            if (upgradeSprite)
            {
                b.ChildOf(upgradeSprite->GetEntityID())
                .At(Anchors::MIDDLE_MIDDLE, UpgradeTextOffset)
                .Font(UiTheme::DefaultFont, UpgradeTextSize)
                .Tint(Colors::PERFECT_BROWN)
                .Text("FORGER")
                .BuildText();

                UiButton* upgradeBtn = b.BuildButton(upgradeSprite);

                upgradeBtn->OnClick.ConnectLambda([this]() {
                    if (!m_CurrentEquip || !m_VillageManagerCache) return;

                    auto& inv = m_VillageManagerCache->GetInventory();
                    bool hasChanged = false;
                    auto& baseStats = const_cast<std::vector<BaseStat>&>(m_CurrentEquip->GetBaseStatistics());

                    for (int i = 0; i < 6; ++i)
                    {
                        std::string sName = m_MappedStats[i];
                        if (sName.empty()) continue;

                        int tier = m_PendingGems[sName];
                        if (tier > 0)
                        {
                            bool consumed = false;
                            if (tier == 1) consumed = inv.ConsumeResource<GreenGemme>(1);
                            else if (tier == 2) consumed = inv.ConsumeResource<RedGemme>(1);
                            else if (tier == 3) consumed = inv.ConsumeResource<BlueGemme>(1);
                            else if (tier == 4) consumed = inv.ConsumeResource<OrangeGemme>(1);
                            else if (tier == 5) consumed = inv.ConsumeResource<PurpleGemme>(1);
                            else if (tier == 6) consumed = inv.ConsumeResource<YellowGemme>(1);

                            if (consumed)
                            {
                                for (auto& bs : baseStats)
                                {
                                    if (bs.Name == sName)
                                    {
                                        bs.Value += bs.Value * (0.05f * static_cast<float>(tier));
                                        break;
                                    }
                                }
                                if (m_StatValuesText[i]) m_StatValuesText[i]->SetTint(Colors::GREEN);
                                if (m_SocketSprites[i]) m_SocketSprites[i]->SetSprite("218x169", "NoaLeGOOOAT");
                                hasChanged = true;
                            }
                        }
                    }


                    if (hasChanged)
                    {
                        m_CurrentEquip->EquipTo(&m_CurrentEquip->GetStatGroup());
                        for (int i = 0; i < 6; ++i)
                        {
                            if (i < baseStats.size())
                            {
                                std::string valStr = std::to_string(baseStats[i].Value);
                                valStr.erase(valStr.find_last_not_of('0') + 1, std::string::npos);
                                if (valStr.back() == '.') valStr.pop_back();
                                if (m_StatValuesText[i]) m_StatValuesText[i]->SetText(valStr);
                            }
                        }
                    }

                    for (auto& [stat, gemTier] : m_PendingGems) gemTier = 0;
                });
            }
            m_DetailCanvas->IsEnable = false;
        }

        for (int i = 0; i <= 6; i++) {
            if (i == 6) { b.ChildOf(canvasId).BuildCanvas(); break; }
            float posX = GemStartX + (i * GemSpacingX);
            std::string gemTexture = "Forge_Cost_Crystal_T" + std::to_string(i + 1);
            UiSprite* gemSprite = b.ChildOf(canvasId)
                                   .At(Anchors::MIDDLE_LEFT, { posX, GemStartY })
                                   .Scale(GemIconScale)
                                   .Sprite("218x169", gemTexture)
                                   .BuildSprite();

            if (!gemSprite) continue;
            UiText* gemValue = b.ChildOf(gemSprite->GetEntityID())
                                .At(Anchors::MIDDLE_MIDDLE, GemTextOffset)
                                .Font(UiTheme::DefaultFont, GemTextFontSize)
                                .Tint(Colors::PERFECT_BROWN)
                                .Text("0")
                                .BuildText();

            if (gemValue) gemValue->BindReactive(m_GemCounts[i]);
        }
        m_ForgeCanvas->IsEnable = false;
    }

    void ForgeBehavior::RefreshInventoryUI()
    {
        if (!m_VillageManagerCache || !m_EmptyCanvas) return;

        const auto& equips = m_VillageManagerCache->GetInventory().GetEquipments();
        int count = 0;

        for (const auto& [id, eq] : equips)
        {
            if (count >= 6) break;

            m_MappedEquips[count] = eq;

            if (m_EquipSprites[count])
            {
                m_EquipSprites[count]->SetSprite("580x157", GetEquipmentTexture(eq));
                m_EquipSprites[count]->IsEnable = true;
            }

            count++;
        }

        for (int i = count; i < 6; ++i)
        {
            m_MappedEquips[i] = nullptr;
            if (m_EquipSprites[i]) m_EquipSprites[i]->IsEnable = false;
        }

        m_EmptyCanvas->IsEnable = (count == 0);
    }

    void ForgeBehavior::BuildEquipDetailPanel(int equipIndex)
    {
        Equipment* pEquip = m_MappedEquips[equipIndex];
        if (!pEquip || !m_DetailCanvas) return;

        m_DetailCanvas->IsEnable = true;
        m_CurrentEquip = pEquip;

        if (m_DetailTitle) m_DetailTitle->SetText(GetEquipmentName(pEquip));

        m_PendingGems.clear();

        if (pEquip->GetBaseStatistics().empty()) pEquip->OnBaseStatistics();

        const auto& baseStats = pEquip->GetBaseStatistics();

        for (int i = 0; i < 6; ++i)
        {
            if (i < baseStats.size())
            {
                std::string sName = baseStats[i].Name;
                m_MappedStats[i] = sName;
                m_PendingGems[sName] = 0;

                std::string valStr = std::to_string(baseStats[i].Value);
                valStr.erase(valStr.find_last_not_of('0') + 1, std::string::npos);
                if (valStr.back() == '.') valStr.pop_back();

                if (m_StatNamesText[i])
                {
                    m_StatNamesText[i]->SetText(sName);
                    m_StatNamesText[i]->IsEnable = true;
                }
                if (m_StatValuesText[i])
                {
                    m_StatValuesText[i]->SetTint(Colors::BLACK);
                    m_StatValuesText[i]->SetText(valStr);
                    m_StatValuesText[i]->IsEnable = true;
                }
                if (m_SocketSprites[i])
                {
                    m_SocketSprites[i]->SetSprite("218x169", "NoaLeGOOOAT");
                    m_SocketSprites[i]->IsEnable = true;
                }
            }
            else
            {
                m_MappedStats[i] = "";
                if (m_StatNamesText[i]) m_StatNamesText[i]->IsEnable = false;
                if (m_StatValuesText[i]) m_StatValuesText[i]->IsEnable = false;
                if (m_SocketSprites[i]) m_SocketSprites[i]->IsEnable = false;
            }
        }
    }

    void ForgeBehavior::OnSocketClicked(int socketIndex)
    {
        std::string statName = m_MappedStats[socketIndex];
        if (statName.empty()) return;

        m_PendingGems[statName]++;
        if (m_PendingGems[statName] > 6) m_PendingGems[statName] = 0;

        if (m_SocketSprites[socketIndex])
        {
            if (m_PendingGems[statName] == 0) m_SocketSprites[socketIndex]->SetSprite("218x169", "NoaLeGOOOAT");
            else m_SocketSprites[socketIndex]->SetSprite("218x169", "Forge_Cost_Crystal_T" + std::to_string(m_PendingGems[statName]));
        }
    }
}