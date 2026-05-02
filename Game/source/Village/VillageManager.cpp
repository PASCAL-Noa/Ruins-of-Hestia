#include "Village/VillageManager.h"

#include <cmath>
#include <memory>

#include "SerialFile.h"
#include "UI/UiSystem.h"
#include "UI/UiAnim.h"
#include "SerialTree.h"
#include "Scenes/SceneVillageV1.h"
#include "Village/Save.h"
#include "Village/Buildings/Farm/Farm.h"
#include "Village/Buildings/House/House.h"
#include "Village/Buildings/Forge/Forge.h"
#include "Village/Buildings/Hospital/Hospital.h"
#include "Village/Buildings/LifeTree/LifeTree.h"

namespace GPC
{

    VillageManager::VillageManager(Scene* scene) : m_ExpeditionCount(0)
    {
        BuildCatalogue();
        m_Inventory.Initialize(false);
        mp_Scene = scene;
    }

    VillageManager::~VillageManager()
    {
    }

    void VillageManager::Update()
    {
        if (!m_IsPlacingNew) {
            HandleInteract(mp_Scene);
        }

    }

    void VillageManager::AdvanceTime()
    {
        m_ExpeditionCount++;

        for (auto& data : m_BuildingsData)
        {
            if (data.LinkedBehavior && data.IsActive)
            {
                data.LinkedBehavior->OnNextCycle();
            }
        }
    }

    void VillageManager::SetAllBuildingsActive(bool active)
    {
        for (auto& data : m_BuildingsData)
        {
            data.IsActive = active;
            if (data.LinkedBehavior)
            {
                data.LinkedBehavior->IsActive = active;
            }
        }
        for (auto behavior : m_BuildingBehaviors)
        {
            if (behavior) behavior->IsActive = active;
        }
    }

    void VillageManager::SyncBuildingActiveState()
    {
        for (auto& data : m_BuildingsData)
        {
            if (!data.LinkedBehavior) continue;
            BuildingBehavior* b = data.LinkedBehavior;
            b->IsActive = data.IsActive;

            if (!b->Building_ || !b->Render3DComponent_) continue;

            const std::string& target = (data.IsActive && !b->Building_->ActivePath.empty())
                                            ? b->Building_->ActivePath
                                            : b->Building_->TexturePath;
            if (b->CurrentBasePath != target)
            {
                b->CurrentBasePath = target;
                b->Render3DComponent_->SetTexture("1220x1683", target);
            }
        }
    }

    void VillageManager::RebuildBehaviorRegistry()
    {
        m_BuildingBehaviors.clear();
        for (auto& data : m_BuildingsData)
        {
            if (data.LinkedBehavior)
            {
                m_BuildingBehaviors.push_back(data.LinkedBehavior);
            }
        }
    }

    namespace
    {
        struct PulseScaleState
        {
            glm::vec3 BaseScale = glm::vec3(1.0f);
            bool      Captured  = false;
        };

        void PushPulseAndSwap(Scene* pScene, BuildingBehavior* b, float startDelay, bool toActive)
        {
            if (!pScene || !b || !b->Render3DComponent_ || !b->Building_) return;

            const EntityID spriteEid = b->Render3DComponent_->GetEntityID();
            auto state = std::make_shared<PulseScaleState>();

            constexpr float kPulseDuration = 0.35f;
            constexpr float kPulseAmount   = 0.08f;

            UiAnimSlot pulse{};
            pulse.Delay    = startDelay;
            pulse.Duration = kPulseDuration;
            pulse.Ease     = Tweening::EasingType::Linear;
            pulse.OnStart  = [pScene, spriteEid, state]()
            {
                if (!pScene->HasComponent<Transform3D>(spriteEid)) return;
                Transform3D* tr = pScene->GetComponent<Transform3D>(spriteEid);
                if (!tr) return;
                state->BaseScale = tr->LocalTransform.GetScale();
                state->Captured  = true;
            };
            pulse.Setter = [pScene, spriteEid, state](float t)
            {
                if (!state->Captured) return;
                if (!pScene->HasComponent<Transform3D>(spriteEid)) return;
                Transform3D* tr = pScene->GetComponent<Transform3D>(spriteEid);
                if (!tr) return;
                const float wave  = std::sin(t * static_cast<float>(GPC_PI));
                const float scale = 1.0f + kPulseAmount * wave;
                tr->LocalTransform.SetScale(state->BaseScale * scale);
            };
            pulse.OnComplete = [pScene, spriteEid, state, b, toActive]()
            {
                if (state->Captured && pScene->HasComponent<Transform3D>(spriteEid))
                {
                    Transform3D* tr = pScene->GetComponent<Transform3D>(spriteEid);
                    if (tr) tr->LocalTransform.SetScale(state->BaseScale);
                }

                if (!b || !b->Render3DComponent_ || !b->Building_) return;
                if (b->IsInteracting) return;

                const std::string& path = toActive ? b->Building_->ActivePath
                                                   : b->Building_->TexturePath;
                b->CurrentBasePath = path;
                b->Render3DComponent_->SetTexture("1220x1683", path);
            };
            UiAnim::PushAnim(*pScene, spriteEid, std::move(pulse));
        }

        std::vector<BuildingBehavior*> CollectSwappable(const std::vector<BuildingBehavior*>& behaviors)
        {
            std::vector<BuildingBehavior*> swappable;
            for (auto behavior : behaviors)
            {
                if (!behavior)                              continue;
                if (!behavior->Building_)                   continue;
                if (behavior->Building_->ActivePath.empty())continue;
                if (!behavior->Render3DComponent_)          continue;
                swappable.push_back(behavior);
            }
            return swappable;
        }
    }

    void VillageManager::StaggerSwapToActiveTextures(float totalDuration)
    {
        auto swappable = CollectSwappable(m_BuildingBehaviors);
        const size_t n = swappable.size();
        if (n == 0) return;

        const float delayPerBuilding = totalDuration / static_cast<float>(n);

        for (size_t i = 0; i < n; ++i)
        {
            const float delay = static_cast<float>(i) * delayPerBuilding;
            PushPulseAndSwap(mp_Scene, swappable[i], delay, true);
        }
    }

    void VillageManager::StaggerSwapToInactiveTextures(float totalDuration)
    {
        auto swappable = CollectSwappable(m_BuildingBehaviors);
        const size_t n = swappable.size();
        if (n == 0) return;

        const float delayPerBuilding = totalDuration / static_cast<float>(n);

        for (size_t i = 0; i < n; ++i)
        {
            const float delay = static_cast<float>(i) * delayPerBuilding;
            PushPulseAndSwap(mp_Scene, swappable[i], delay, false);
        }
    }

    int VillageManager::GetExpeditionCount() const
    {
        return m_ExpeditionCount;
    }

    bool VillageManager::LoadVillageData()
    {
        SerialTree tree;
        if (SerialFile::Load(Saves::SAVE_FILE_PATH, tree) != ErrorType::SUCCESS)
        {
            LoadDefaultVillage();
            return true;
        }

        if (tree.HasKey(Saves::KEY_EXPEDITIONS))
        {
            m_ExpeditionCount = *tree[Saves::KEY_EXPEDITIONS].Get<int>();
        }

        LoadBuildingsFromTree(tree);
        m_Inventory.LoadFromFile(tree);
        return false;
    }

    void VillageManager::SaveVillageData()
    {
        SerialTree tree;

        tree[Saves::KEY_EXPEDITIONS].Set(&m_ExpeditionCount, 1);

        if (!m_BuildingsData.empty())
        {
            tree[Saves::KEY_BUILDINGS].Set(m_BuildingsData.data(), m_BuildingsData.size());
        }

        m_Inventory.SaveToFile(tree);
        SerialFile::Save(Saves::SAVE_FILE_PATH, tree);
    }

    void VillageManager::Reset()
    {
        m_BuildingsData.clear();
        m_Inventory.Clear();

        LoadDefaultVillage();
        SaveVillageData();
    }

    bool VillageManager::CanAfford(BuildingType type) const
    {
        const auto& costs = GetBuildingTemplate(type).Infos->Upgrades;

        for (const auto& cost : costs)
        {
            if (!m_Inventory.HasResourceAmount(cost.ResourceId, cost.Amount))
            {
                return false;
            }
        }
        return true;
    }

    void VillageManager::PayForBuilding(BuildingType type)
    {
        const auto& costs = GetBuildingTemplate(type).Infos->Upgrades;

        for (auto upgrade : costs) {
            m_Inventory.ConsumeResource(upgrade.ResourceId, upgrade.Amount);
        }

    }

    void VillageManager::LoadDefaultVillage()
    {
        m_ExpeditionCount = 0;

        GiveDefaultResource();
        // GiveDefaultEquipments();  //TODO Ca va la mais on save pas les equipments
        RecalculateStashCache();
    }

    void VillageManager::GiveDefaultResource()
    {

        m_Inventory.AddResource<Wood<ResourceTier::T1>>(500);
        m_Inventory.AddResource<Stone<ResourceTier::T1>>(200);
        m_Inventory.AddResource<Fiber<ResourceTier::T1>>(50);
        m_Inventory.AddResource<Food>(120);
        m_Inventory.AddResource<PurpleGemme>(3);
        m_Inventory.AddResource<RedGemme>(2);

    }

    void VillageManager::GiveDefaultEquipments()
    {
        auto pushNew = [&](Equipment* pEq)
        {
            pEq->OnBaseStatistics();
            pEq->ReRollTraits();
            m_Inventory.AddEquipment(pEq);
        };

        pushNew(new Gauntlet(1));
        pushNew(new Axe(1));
        pushNew(new Helmet(1));
        pushNew(new ChestPlate(1));
        pushNew(new Gloves(1));
        pushNew(new Boots(1));
    }

    void VillageManager::LoadBuildingsFromTree(const SerialTree& tree)
    {
        m_BuildingsData.clear();

        if (!tree.HasKey(Saves::KEY_BUILDINGS))
            return;

        const SerialTree& buildingsNode = tree[Saves::KEY_BUILDINGS];
        uint64_t count = buildingsNode.Count<BuildingData>();
        BuildingData* pData = buildingsNode.Get<BuildingData>();

        m_BuildingsData.resize(count);
        std::memcpy(m_BuildingsData.data(), pData, sizeof(BuildingData) * count);
        RecalculateStashCache();
    }

    void VillageManager::BuildCatalogue()
    {
        Forge* tempForge = new Forge();
        m_BuildingDatabase[BuildingType::Forge] = {
            tempForge->GetTexturePath(),
            tempForge->GetSize(),
            tempForge->GetScale(),
            tempForge
        };

        Hospital* tempHospital = new Hospital();
        m_BuildingDatabase[BuildingType::Hospital] = {
            tempHospital->GetTexturePath(),
            tempHospital->GetSize(),
            tempHospital->GetScale(),
            tempHospital
        };

        House* tempHouse = new House();
        m_BuildingDatabase[BuildingType::House] = {
            tempHouse->GetTexturePath(),
            tempHouse->GetSize(),
            tempHouse->GetScale(),
            tempHouse
        };

        Farm* tempFarm = new Farm;
        m_BuildingDatabase[BuildingType::Farm] = {
            tempFarm->GetTexturePath(),
            tempFarm->GetSize(),
            tempFarm->GetScale(),
            tempFarm
        };

        LifeTree* tempLifeTree = new LifeTree();
        m_BuildingDatabase[BuildingType::LifeTree] = {
            tempLifeTree->GetTexturePath(),
            tempLifeTree->GetSize(),
            tempLifeTree->GetScale(),
            tempLifeTree
        };

    }

    void VillageManager::RecalculateStashCache()
    {
        m_StashedCounts.clear();
        for (const auto& b : m_BuildingsData)
        {
            if (b.IsStashed)
            {
                m_StashedCounts[b.Type]++;
            }
        }
    }

    void VillageManager::HandleInteract(Scene* scene) const
    {
        if (!mp_Controller->IsBuilding() && !mp_Controller->IsEditing() && Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_LEFT)) {

            auto* ui = scene->GetSystem<UiSystem>();
            if (ui->HasPressedButton()) return;

            RayHit hit = scene->GetCameraRay({Inputs::GetMouseX(), Inputs::GetMouseY()});
            if (hit.ContactPoint.x < 1E10) {
                bool isInteract = false;
                BuildingBehavior* behavior = nullptr;
                for (auto behaviors : m_BuildingBehaviors) {
                    if (hit.HitEntity == behaviors->GetEntityID()) {
                        if (!behaviors->IsInteracting) {
                            behaviors->OnInteract();
                            behavior = behaviors;
                            isInteract = true;
                        }
                    }
                }

                if (!isInteract) {
                    for (const auto behaviors : m_BuildingBehaviors) {
                        //if (behaviors->IsInteracting)
                        //{
                        //    behaviors->CloseOpened();
                        //}
                    }
                }

                if (isInteract) {
                    for (auto behaviors : m_BuildingBehaviors) {
                        if (behaviors != behavior) {
                            if (behaviors->IsInteracting)
                            {
                                behaviors->CloseOpened();
                            }
                        }
                    }
                    mp_Controller->StartInspection();
                } else {
                    for (auto behaviors : m_BuildingBehaviors) {
                        behaviors->CloseOpened();
                    }
                    mp_Controller->EndInspection();
                    mp_VillageSFXList[1]->Play();
                }
            }
        }
    }

    int VillageManager::GetStashedCount(BuildingType type) const
    {
        auto it = m_StashedCounts.find(type);
        if (it != m_StashedCounts.end())
        {
            return it->second;
        }
        return 0;
    }

    void VillageManager::StartPlacement(BuildingType type)
    {
        m_PlacementType = type;
        m_IsPlacingNew = true;
        m_PlacementTimer = 0.0f;
    }

    void VillageManager::UpdatePlacement(Scene* scene, VillageController* controller, float dt)
    {
        if (!m_IsPlacingNew || !controller) return;

        controller->StartBuild();
        m_PlacementTimer += dt;

        Editor* editor = controller->GetEditor();
        if (!editor) return;

        RayHit hit = scene->GetCameraRay({Inputs::GetMouseX(), Inputs::GetMouseY()});
        if (hit.ContactPoint.x < 1E10)
        {
            auto* camera = scene->GetActiveCamera();
            auto* camTransform = scene->GetComponent<Transform3D>(camera->GetEntityID());
            const glm::vec3 rayDir = -camTransform->LocalTransform.GetForward();
            const glm::vec2 worldXZ = mp_Map->ProjectRayToTiltedGround(hit.ContactPoint, rayDir);
            editor->SetCursorAt({worldXZ.x, worldXZ.y, 0.0f});
            controller->StartBuild();
        }

        if (m_PlacementTimer > 0.2f)
        {
            if (Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_LEFT))
            {
                glm::ivec2 pos = editor->GetCursor();
                glm::ivec2 size = GetBuildingTemplate(m_PlacementType).Size;

                if (mp_Map->IsSpaceFree(pos, size) && CanAfford(m_PlacementType))
                {

                    BuildingBehavior* behavior = AddBuilding(m_PlacementType, pos);
                    PayForBuilding(m_PlacementType);
                    m_IsPlacingNew = false;
                    controller->EndBuild();

                }
            }
        }
    }

    std::vector<AudioSource *> & VillageManager::GetAudioSources()
    {
        return mp_VillageSFXList;
    }


    const VillageManager::BuildingTemplate& VillageManager::GetBuildingTemplate(BuildingType type) const
    {
        auto it = m_BuildingDatabase.find(type);
        if (it != m_BuildingDatabase.end())
        {
            return it->second;
        }

        static BuildingTemplate defaultTemplate = { "Cube", { 1, 1 } };
        return defaultTemplate;
    }

    std::vector<BuildingData> & VillageManager::GetBuildingsData()
    {
        return m_BuildingsData;
    }

    size_t VillageManager::GetBuildingsCount() const
    {
        return m_BuildingsData.size();
    }

    BuildingBehavior* VillageManager::AddBuilding(BuildingType type, glm::ivec2 position)
    {

        BuildingBehavior* behavior = nullptr;
        if (OnBuildingAddedCallback)
        {
            behavior = OnBuildingAddedCallback(type, position);
            m_BuildingBehaviors.push_back(behavior);
        }
        m_BuildingsData.push_back({ behavior, type, position, false });

        return behavior;
    }

    void VillageManager::UpdateBuildingPosition(size_t index, glm::ivec2 newPosition)
    {
        if (index < m_BuildingsData.size())
            m_BuildingsData[index].Position = newPosition;
    }


    void VillageManager::StashBuildingAt(glm::ivec2 position)
    {
        for (auto& b : m_BuildingsData)
        {
            if (b.Position == position && !b.IsStashed)
            {
                b.IsStashed = true;

                b.LinkedBehavior->Render3DComponent_->IsEnable = false;
                b.LinkedBehavior->Collision3DComponent_->IsEnable = false;

                m_StashedCounts[b.Type]++;
                GPC_INFO << "[INVENTAIRE] Batiment range. (En stock : " << m_StashedCounts[b.Type] << ")" << ENDL;
                if (OnBuildingStashedCallback)
                    OnBuildingStashedCallback(position, b.Type);
                return;
            }
        }
    }

    bool VillageManager::UnstashBuilding(BuildingType type, glm::ivec2 newPosition)
    {
        for (auto& b : m_BuildingsData)
        {
            if (b.Type == type && b.IsStashed)
            {
                b.IsStashed = false;
                b.Position = newPosition;

                b.LinkedBehavior->Render3DComponent_->IsEnable = true;
                b.LinkedBehavior->Collision3DComponent_->IsEnable = true;

                Transform3D* transformSprite = mp_Scene->GetComponent<Transform3D>(b.LinkedBehavior->Render3DComponent_->GetEntityID());
                Transform3D* transform = mp_Scene->GetComponent<Transform3D>(b.LinkedBehavior->Collision3DComponent_->GetEntityID());

                const auto* bld = b.LinkedBehavior->Building_;
                const float bldScale = bld->GetScale();
                const float spriteHeight = static_cast<float>(bld->GetSize().x) * (1683.0f / 1220.0f) * 1.4f * bldScale;
                transformSprite->LocalTransform.SetPosition({b.Position.x, transformSprite->LocalTransform.GetPosition().y, b.Position.y});
                transform->LocalTransform.SetPosition({b.Position.x + bld->GetSize().x/2.0f, transform->LocalTransform.GetPosition().y, b.Position.y + bld->GetSize().y/2.0f});
                transform->LocalTransform.SetScale(glm::vec3(bld->GetSize().x * 0.8f, spriteHeight * 0.3f, bld->GetSize().y * 0.8f));
                b.LinkedBehavior->Collision3DComponent_->Offset = glm::vec3(0.0f, spriteHeight * 0.15f - 0.5f, 0.0f);

                m_StashedCounts[type]--;

                GPC_INFO << "[INVENTAIRE] Batiment place. (Reste : " << m_StashedCounts[type] << ")" << ENDL;
                if (OnBuildingUnstashedCallback)
                    OnBuildingUnstashedCallback(b.LinkedBehavior, newPosition);

                mp_Map->SetGridActive(true);

                return true;
            }
        }
        return false;
    }


}