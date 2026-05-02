#include "Expedition/ResourceBehavior.h"

#include "ParticleSystem.h"
#include "Random.h"
#include "Scene.h"
#include "Expedition/ResourceFeedBackAnimation.h"
#include "Expedition/ResourceSplashEffect.h"
#include "Scenes/SceneDefault.h"
#include "UI/UiBuilder.h"

namespace GPC {

    bool ResourceBehavior::TryCollect(ResourceGeneratorBehavior::ResourceComponents *pResource, Scene *pScene, float time) {
        if (m_IsBeingCollected) return false;
        m_BaseScaleResource1 = pResource->pTransform1->LocalTransform.GetScale();
        m_BaseScaleResources2 = pResource->pTransform2->LocalTransform.GetScale();

        ResourceID id = GetResourceID();
        pManager->pInventory->AddResource(id, m_ResourceQte);
        // update ui
        m_HitRemaining--;

        // TODO : Fix la deletion des UIs
        if (pScene->GetEntityCount() < 5000)
            CreateUIFeedBackResource(pScene);

        TweenConfig<float> config{};
        config.Duration = COOLDOWN_COLLECT_DURATION;
        config.Start = 0.0f;
        config.End = 1.0f;
        config.Ease = Tweening::EasingType::EaseOutBounce;
        config.Setter = [pResource](float value) {
            float t = -4.0f * value * value + 4.0f * value;

            glm::vec3 scale_tronc = pResource->pBehavior->m_BaseScaleResource1 + SCALE_OFFSET_RESOURCE1 * t;
            pResource->pTransform1->LocalTransform.SetScale(scale_tronc);
            pResource->pTransform1->LocalTransform.SetPosition(
                pResource->pTransform1->LocalTransform.GetPosition().x,
                scale_tronc.z * 0.5f,
                pResource->pTransform1->LocalTransform.GetPosition().z
            );

            glm::vec3 scale_leaves = pResource->pBehavior->m_BaseScaleResources2 + SCALE_OFFSET_RESOURCE2 * t;
            pResource->pTransform2->LocalTransform.SetScale(scale_leaves);
            pResource->pTransform2->LocalTransform.SetPosition(
                pResource->pTransform2->LocalTransform.GetPosition().x,
                scale_leaves.z * 0.5f,
                pResource->pTransform2->LocalTransform.GetPosition().z
            );
        };
        pResource->pTweening->AddTween(config);

        m_TimeShouldBeCollectable = time + COOLDOWN_COLLECT_DURATION;
        m_IsBeingCollected = true;
        return true;
    }


    void ResourceBehavior::OnUpdate(const BehaviorUpdateContext *pCtx) {
        Behavior::OnUpdate(pCtx);

        if (m_IsBeingCollected && pCtx->pClock->GetTime() >= m_TimeShouldBeCollectable) {
            m_IsBeingCollected = false;
            if (m_HitRemaining == 0) {
                pManager->DestroyResource(pComponents, pCtx->pScene);
            }
        }

    }

    void ResourceBehavior::InitAs(Tier resource_tier, ResourceType resource_type, Scene* pScene) {
        m_ResourceTier = resource_tier;
        m_ResourceType = resource_type;

        switch (m_ResourceType) {
            case ResourceType::WOOD:
                InitVisualAsWood(pScene);
                break;
            default:
            case ResourceType::STONE:
                InitVisualAsStone(pScene);
                break;
            case ResourceType::GREEN_GEMME:
                InitVisualAsGreenGem(pScene);
                break;
            case ResourceType::YELLOW_GEMME:
                InitVisualAsYellowGem(pScene);
                break;
            case ResourceType::RED_GEMME:
                InitVisualAsRedGem(pScene);
                break;
            case ResourceType::BLUE_GEMME:
                InitVisualAsBlueGem(pScene);
                break;
            case ResourceType::ORANGE_GEMME:
                InitVisualAsOrangeGem(pScene);
                break;
            case ResourceType::PURPLE_GEMME:
                InitVisualAsPurpleGem(pScene);
                break;

        }
    }

    void ResourceBehavior::InitVisualAsWood(Scene* pScene) {
        uint8_t tronc_index = rand() % GPC::TRONC_COUNT;
        uint8_t leaves_color_index = rand() % GPC::LEAVES_COLOR_COUNT;

        m_ResourceQte = 5 + rand() % 15;

        pComponents->UseSprite1 = true;
        pComponents->UseSprite2 = true;

        pComponents->pRenderer1->SetTexture("ResourceAndDecor", "Tronc" + std::to_string(tronc_index));
        pComponents->pRenderer1->UseShadow = true;
        pComponents->pRenderer1->IsEnable = true;
        pComponents->pTransform1->LocalTransform.SetScale(TREE_SCALE.x * GENERAL_SCALE_EXPEDITION, 0.05f, TREE_SCALE.y * GENERAL_SCALE_EXPEDITION);
        pComponents->pTransform1->LocalTransform.SetPosition(
            pComponents->pTransform1->LocalTransform.GetPosition().x,
            TREE_SCALE.y * GENERAL_SCALE_EXPEDITION * 0.5f,
            pComponents->pTransform1->LocalTransform.GetPosition().z
        );

        pComponents->pRenderer2->SetTexture("ResourceAndDecor", "Leaves" + std::to_string(leaves_color_index) + "_" + std::to_string(tronc_index));
        pComponents->pRenderer2->UseShadow = true;
        pComponents->pRenderer2->IsEnable = true;
        pComponents->pTransform2->LocalTransform.SetScale(TREE_SCALE.x * GENERAL_SCALE_EXPEDITION, 0.05f, TREE_SCALE.y * GENERAL_SCALE_EXPEDITION);
        pComponents->pTransform2->LocalTransform.SetPosition(
            pComponents->pTransform2->LocalTransform.GetPosition().x,
            TREE_SCALE.y * GENERAL_SCALE_EXPEDITION * 0.5f,
            pComponents->pTransform2->LocalTransform.GetPosition().z
        );

        // pComponents->pRenderer2->RenderMaterial.Tint = glm::vec4{
        //     GPC::Random::Float(0.0f, 1.0f),
        //     GPC::Random::Float(0.0f, 1.0f),
        //     GPC::Random::Float(0.0f, 1.0f),
        //     GPC::Random::Float(0.0f, 0.05f)
        // };

    }

    void ResourceBehavior::InitVisualAsStone(Scene* pScene) {
        uint8_t cailloux_index = rand() % GPC::CAILLOUX_COUNT;

        m_ResourceQte = 5 + rand() % 15;

        pComponents->UseSprite1 = false;
        pComponents->UseSprite2 = true;

        pComponents->pRenderer1->IsEnable = false;

        pComponents->pRenderer2->IsEnable = true;
        pComponents->pRenderer2->SetTexture("ResourceAndDecor", "Cailloux" + std::to_string(cailloux_index));
        pComponents->pRenderer2->UseShadow = true;
        pComponents->pTransform2->LocalTransform.SetScale(CAILLOUX_SCALE.x * GENERAL_SCALE_EXPEDITION, 0.05f, CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION);
        pComponents->pTransform2->LocalTransform.SetPosition(
            pComponents->pTransform2->LocalTransform.GetPosition().x,
            CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION * 0.5f,
            pComponents->pTransform2->LocalTransform.GetPosition().z
        );
    }

    void ResourceBehavior::InitVisualAsBlueGem(Scene *pScene) {
        uint8_t gem_index = rand() % GPC::GEM_COUNT;

        m_ResourceQte = 1 + rand() % 2;

        pComponents->UseSprite1 = false;
        pComponents->UseSprite2 = true;

        pComponents->pRenderer1->IsEnable = false;

        pComponents->pRenderer2->IsEnable = true;
        pComponents->pRenderer2->SetTexture("ResourceAndDecor", "BlueGem" + std::to_string(gem_index));
        pComponents->pRenderer2->UseShadow = true;
        pComponents->pTransform2->LocalTransform.SetScale(CAILLOUX_SCALE.x * GENERAL_SCALE_EXPEDITION, 0.05f, CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION);
        pComponents->pTransform2->LocalTransform.SetPosition(
            pComponents->pTransform2->LocalTransform.GetPosition().x,
            CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION * 0.5f,
            pComponents->pTransform2->LocalTransform.GetPosition().z
        );
    }

    void ResourceBehavior::InitVisualAsRedGem(Scene *pScene) {
        uint8_t gem_index = rand() % GPC::GEM_COUNT;

        m_ResourceQte = 1 + rand() % 2;

        pComponents->UseSprite1 = false;
        pComponents->UseSprite2 = true;

        pComponents->pRenderer1->IsEnable = false;

        pComponents->pRenderer2->IsEnable = true;
        pComponents->pRenderer2->SetTexture("ResourceAndDecor", "RedGem" + std::to_string(gem_index));
        pComponents->pRenderer2->UseShadow = true;
        pComponents->pTransform2->LocalTransform.SetScale(CAILLOUX_SCALE.x * GENERAL_SCALE_EXPEDITION, 0.05f, CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION);
        pComponents->pTransform2->LocalTransform.SetPosition(
            pComponents->pTransform2->LocalTransform.GetPosition().x,
            CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION * 0.5f,
            pComponents->pTransform2->LocalTransform.GetPosition().z
        );
    }

    void ResourceBehavior::InitVisualAsYellowGem(Scene *pScene) {
        uint8_t gem_index = rand() % GPC::GEM_COUNT;

        m_ResourceQte = 1 + rand() % 2;

        pComponents->UseSprite1 = false;
        pComponents->UseSprite2 = true;

        pComponents->pRenderer1->IsEnable = false;

        pComponents->pRenderer2->IsEnable = true;
        pComponents->pRenderer2->SetTexture("ResourceAndDecor", "YellowGem" + std::to_string(gem_index));
        pComponents->pRenderer2->UseShadow = true;
        pComponents->pTransform2->LocalTransform.SetScale(CAILLOUX_SCALE.x * GENERAL_SCALE_EXPEDITION, 0.05f, CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION);
        pComponents->pTransform2->LocalTransform.SetPosition(
            pComponents->pTransform2->LocalTransform.GetPosition().x,
            CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION * 0.5f,
            pComponents->pTransform2->LocalTransform.GetPosition().z
        );
    }

    void ResourceBehavior::InitVisualAsGreenGem(Scene *pScene) {
        uint8_t gem_index = rand() % GPC::GEM_COUNT;

        m_ResourceQte = 1 + rand() % 2;

        pComponents->UseSprite1 = false;
        pComponents->UseSprite2 = true;

        pComponents->pRenderer1->IsEnable = false;

        pComponents->pRenderer2->IsEnable = true;
        pComponents->pRenderer2->SetTexture("ResourceAndDecor", "GreenGem" + std::to_string(gem_index));
        pComponents->pRenderer2->UseShadow = true;
        pComponents->pTransform2->LocalTransform.SetScale(CAILLOUX_SCALE.x * GENERAL_SCALE_EXPEDITION, 0.05f, CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION);
        pComponents->pTransform2->LocalTransform.SetPosition(
            pComponents->pTransform2->LocalTransform.GetPosition().x,
            CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION * 0.5f,
            pComponents->pTransform2->LocalTransform.GetPosition().z
        );
    }

    void ResourceBehavior::InitVisualAsOrangeGem(Scene *pScene) {
        uint8_t gem_index = rand() % GPC::GEM_COUNT;

        m_ResourceQte = 1 + rand() % 2;

        pComponents->UseSprite1 = false;
        pComponents->UseSprite2 = true;

        pComponents->pRenderer1->IsEnable = false;

        pComponents->pRenderer2->IsEnable = true;
        pComponents->pRenderer2->SetTexture("ResourceAndDecor", "OrangeGem" + std::to_string(gem_index));
        pComponents->pRenderer2->UseShadow = true;
        pComponents->pTransform2->LocalTransform.SetScale(CAILLOUX_SCALE.x * GENERAL_SCALE_EXPEDITION, 0.05f, CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION);
        pComponents->pTransform2->LocalTransform.SetPosition(
            pComponents->pTransform2->LocalTransform.GetPosition().x,
            CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION * 0.5f,
            pComponents->pTransform2->LocalTransform.GetPosition().z
        );
    }

    void ResourceBehavior::InitVisualAsPurpleGem(Scene *pScene) {
        uint8_t gem_index = rand() % GPC::GEM_COUNT;

        m_ResourceQte = 1 + rand() % 2;

        pComponents->UseSprite1 = false;
        pComponents->UseSprite2 = true;

        pComponents->pRenderer1->IsEnable = false;

        pComponents->pRenderer2->IsEnable = true;
        pComponents->pRenderer2->SetTexture("ResourceAndDecor", "PurpleGem" + std::to_string(gem_index));
        pComponents->pRenderer2->UseShadow = true;
        pComponents->pTransform2->LocalTransform.SetScale(CAILLOUX_SCALE.x * GENERAL_SCALE_EXPEDITION, 0.05f, CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION);
        pComponents->pTransform2->LocalTransform.SetPosition(
            pComponents->pTransform2->LocalTransform.GetPosition().x,
            CAILLOUX_SCALE.y * GENERAL_SCALE_EXPEDITION * 0.5f,
            pComponents->pTransform2->LocalTransform.GetPosition().z
        );
    }

    float ResourceBehavior::GetIndicatorOffsetY() {
        switch (m_ResourceType) {
            case ResourceType::WOOD: return TREE_OFFSET_Y;
            default: return CAILLOUX_OFFSET_Y;
            case ResourceType::STONE: return CAILLOUX_OFFSET_Y;
            case ResourceType::GREEN_GEMME: return CAILLOUX_OFFSET_Y;
            case ResourceType::YELLOW_GEMME: return CAILLOUX_OFFSET_Y;
            case ResourceType::RED_GEMME: return CAILLOUX_OFFSET_Y;
            case ResourceType::BLUE_GEMME: return CAILLOUX_OFFSET_Y;
            case ResourceType::ORANGE_GEMME: return CAILLOUX_OFFSET_Y;
            case ResourceType::PURPLE_GEMME: return CAILLOUX_OFFSET_Y;
        }
    }

#define RESOURCE_ID(Resource, tier) switch (tier) { default: case 1: return Resource<ResourceTier::T1>::ResourceId; case 2: return Resource<ResourceTier::T2>::ResourceId; case 3: return Resource<ResourceTier::T3>::ResourceId; case 4: return Resource<ResourceTier::T4>::ResourceId; case 5: return Resource<ResourceTier::T5>::ResourceId; }

    ResourceID ResourceBehavior::GetResourceID() {
        switch (m_ResourceType) {
            case ResourceType::WOOD: RESOURCE_ID(Wood, m_ResourceTier);
            default:
            case ResourceType::STONE: RESOURCE_ID(Stone, m_ResourceTier);
            case ResourceType::GREEN_GEMME: return GreenGemme::ResourceId;
            case ResourceType::YELLOW_GEMME: return YellowGemme::ResourceId;
            case ResourceType::RED_GEMME: return RedGemme::ResourceId;
            case ResourceType::BLUE_GEMME: return BlueGemme::ResourceId;
            case ResourceType::ORANGE_GEMME: return OrangeGemme::ResourceId;
            case ResourceType::PURPLE_GEMME: return PurpleGemme::ResourceId;
        }
    }

    void ResourceBehavior::CreateUIFeedBackResource(Scene *pScene) {
        auto feedback = pScene->AddBehavior<ResourceFeedBackAnimation>(pScene->CreateEntityAs3D());
        feedback->ResourcePosition = pComponents->pTransform1->GetWorldPosition();
        feedback->pCanvas = pManager->pCanvas;

        switch (m_ResourceType) {
            case ResourceType::WOOD: break;
                feedback->ResourceUITextureName = "Pickup_Wood";
            default:
            case ResourceType::STONE:
                feedback->ResourceUITextureName = "Pickup_Stone";
                break;
            case ResourceType::GREEN_GEMME:
                feedback->ResourceUITextureName = "Pickup_Green";
                break;
            case ResourceType::YELLOW_GEMME:
                feedback->ResourceUITextureName = "Pickup_Yellow";
                break;
            case ResourceType::RED_GEMME:
                feedback->ResourceUITextureName = "Pickup_Red";
                break;
            case ResourceType::BLUE_GEMME:
                feedback->ResourceUITextureName = "Pickup_Blue";
                break;
            case ResourceType::ORANGE_GEMME:
                feedback->ResourceUITextureName = "Pickup_Orange";
                break;
            case ResourceType::PURPLE_GEMME:
                feedback->ResourceUITextureName = "Pickup_Purple";
                break;
        }
    }

#undef RESOURCE_ID

} // GPC