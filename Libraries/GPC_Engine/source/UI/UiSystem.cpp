#include "UI/UiSystem.h"

#include <algorithm>

#include "Assets.h"
#include "ComponentsBlockManager.h"
#include "Inputs.h"
#include "RenderWindow.h"
#include "Scene.h"
#include "TextureArray.h"
#include "TransformComponents.h"
#include "Clock.h"
#include "Tweens.h"
#include "UI/Raycastable.h"
#include "UI/UiAnimComponent.h"
#include "UI/UiButton.h"
#include "UI/UiCanvas.h"
#include "UI/UiLayout.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"

namespace GPC
{
    static bool TestRaycastable(const Raycastable& ray, glm::vec2 mouse, glm::vec2 position, glm::vec2 size)
    {
        switch (ray.Shape)
        {
            case RaycastShape::Rect:
            {
                const glm::vec2 inset = ray.RectInset;
                return mouse.x >= position.x + inset.x
                    && mouse.y >= position.y + inset.y
                    && mouse.x <= position.x + size.x - inset.x
                    && mouse.y <= position.y + size.y - inset.y;
            }
            case RaycastShape::Circle:
            {
                const glm::vec2 center = position + size * 0.5f;
                const glm::vec2 d      = mouse - center;
                const float     r      = ray.Radius > 0.0f ? ray.Radius : std::min(size.x, size.y) * 0.5f;
                return (d.x * d.x + d.y * d.y) <= r * r;
            }
            case RaycastShape::None:
            default:
                return false;
        }
    }

    void UiSystem::OnUpdate()
    {
        System::OnUpdate();
        if (m_GlobalEnable == false) return;

        if (!m_Ctx.pScene) return;

        m_LeftClickThisFrame   = Inputs::IsButtonPress(Inputs::ButtonCode::BUTTON_LEFT);
        m_LeftReleaseThisFrame = Inputs::IsButtonRelease(Inputs::ButtonCode::BUTTON_LEFT);

        UpdateAnims();

        UpdateTexts();
        UpdateSprites();
        UpdateCanvases();

        for (EntityID root : m_LayoutRoots)
        {
            auto* layout = m_Ctx.pScene->GetComponent<UiLayout>(root);
            if (!layout || !layout->IsDirty) continue;
            ResolveLayoutFor(*m_Ctx.pScene, root);
            layout->IsDirty = false;
        }

        CallHighestButton();
        Draw();
    }

    void UiSystem::OnLateUpdate()
    {
        System::OnLateUpdate();
        if (m_GlobalEnable == false) return;
        mp_HighestButton = nullptr;
    }

    void UiSystem::UpdateAnims()
    {
        if (!m_Ctx.pScene || !m_Ctx.pClock) return;

        auto block = m_Ctx.pScene->GetComponentBlock<UiAnimComponent>();
        if (!block) return;

        const float dt = static_cast<float>(m_Ctx.pClock->GetDeltaTime());
        UiAnimComponent* items = block->ComponentData();
        EntityID* entities = block->EntitiesData();
        const uint32_t count = block->GetEntityCount();

        for (uint32_t i = 0; i < count; ++i)
        {
            if (entities[i] == MAX_ENTITIES) continue;
            if (!items[i].PlaybackActive) continue;
            auto& slots = items[i].Active;
            if (slots.empty()) continue;

            const float compRate = items[i].PlaybackRate;

            std::erase_if(slots, [dt, compRate](UiAnimSlot& s)
            {
                const float effDt = dt * compRate * s.PlaybackRate;

                if (!s.Started)
                {
                    if (s.Delay > 0.0f)
                    {
                        s.Delay -= effDt;
                        if (s.Delay > 0.0f) return false;
                    }
                    s.Started = true;
                    if (s.OnStart) s.OnStart();
                }

                s.Elapsed += effDt;

                const auto applyT = [&](float rawT)
                {
                    const auto easeFunc = Tweening::GetFunction(s.Ease);
                    const float easedT  = easeFunc ? easeFunc(rawT) : rawT;
                    const float finalT  = s.Reversed ? (1.0f - easedT) : easedT;
                    if (s.Setter)   s.Setter(finalT);
                    if (s.OnUpdate) s.OnUpdate(finalT);
                };

                if (s.Duration <= 0.0f)
                {
                    applyT(1.0f);
                    if (s.OnComplete) s.OnComplete();
                    return true;
                }

                const float rawT = std::clamp(s.Elapsed / s.Duration, 0.0f, 1.0f);
                applyT(rawT);

                if (s.Elapsed >= s.Duration)
                {
                    if (s.OnComplete) s.OnComplete();

                    if (s.Loop != UiAnimLoop::None && s.RepeatCount != 0)
                    {
                        if (s.RepeatCount > 0) --s.RepeatCount;
                        s.Elapsed = 0.0f;
                        if (s.Loop == UiAnimLoop::PingPong) s.Reversed = !s.Reversed;
                        return false;
                    }
                    return true;
                }
                return false;
            });
        }
    }

    void UiSystem::RegisterLayoutRoot(EntityID eid)
    {
        if (std::find(m_LayoutRoots.begin(), m_LayoutRoots.end(), eid) == m_LayoutRoots.end())
        {
            m_LayoutRoots.push_back(eid);
        }
    }

    void UiSystem::UnregisterLayoutRoot(EntityID eid)
    {
        m_LayoutRoots.erase(
            std::remove(m_LayoutRoots.begin(), m_LayoutRoots.end(), eid),
            m_LayoutRoots.end());
    }

    void UiSystem::OnEntityDestroyed(EntityID eid)
    {
        UnregisterLayoutRoot(eid);

        if (mp_HighestButton && m_Ctx.pScene)
        {
            auto* btn = m_Ctx.pScene->GetBehavior<UiButton>(eid);
            if (btn && btn == mp_HighestButton) mp_HighestButton = nullptr;
        }
        if (mp_PressedButton && m_Ctx.pScene)
        {
            auto* btn = m_Ctx.pScene->GetBehavior<UiButton>(eid);
            if (btn && btn == mp_PressedButton) mp_PressedButton = nullptr;
        }
    }

    void UiSystem::UpdateTexts()
    {
        auto block = m_Ctx.pScene->GetComponentBlock<UiText>();
        UiText* items = block->ComponentData();
        EntityID* entities = block->EntitiesData();
        const uint32_t count = block->GetEntityCount();

        for (uint32_t i = 0; i < count; ++i)
        {
            if (entities[i] == MAX_ENTITIES) continue;
            if (!items[i].IsEnable) continue;
            if (items[i].m_Updated) items[i].CalcElementSize();
            items[i].m_ScaledSize = items[i].m_Size * m_GlobalRatio * items[i].m_FontSizeRatio;

            if (items[i].IsButton && !IsAncestorDisabled(items[i].GetEntityID()))
                HandleButton(items[i].GetEntityID());
        }
    }

    void UiSystem::UpdateSprites()
    {
        auto block = m_Ctx.pScene->GetComponentBlock<UiSprite>();
        UiSprite* items = block->ComponentData();
        EntityID* entities = block->EntitiesData();
        const uint32_t count = block->GetEntityCount();

        for (uint32_t i = 0; i < count; ++i)
        {
            if (entities[i] == MAX_ENTITIES) continue;
            if (!items[i].IsEnable) continue;
            items[i].CalcElementSize();
            items[i].m_ScaledSize = items[i].m_Size * items[i].m_Scale * m_GlobalRatio;

            if (items[i].IsButton && !IsAncestorDisabled(items[i].GetEntityID()))
                HandleButton(items[i].GetEntityID());
        }
    }

    void UiSystem::UpdateCanvases()
    {
        auto block = m_Ctx.pScene->GetComponentBlock<UiCanvas>();
        UiCanvas* items = block->ComponentData();
        EntityID* entities = block->EntitiesData();
        const uint32_t count = block->GetEntityCount();

        for (uint32_t i = 0; i < count; ++i)
        {
            if (entities[i] == MAX_ENTITIES) continue;
            if (!items[i].IsEnable) continue;

            if (items[i].IsFitToScreen())
            {
                const glm::vec2 windowSize = glm::vec2(m_Ctx.pWindow->GetSize());
                items[i].m_ScaledSize = windowSize;
                if (items[i].m_Size.x > 0.0f) items[i].m_Scale.x = windowSize.x / items[i].m_Size.x;
                if (items[i].m_Size.y > 0.0f) items[i].m_Scale.y = windowSize.y / items[i].m_Size.y;
            }
            else
            {
                items[i].m_ScaledSize = items[i].m_Size * items[i].m_Scale * m_GlobalRatio;
            }

            if (items[i].IsButton && !IsAncestorDisabled(items[i].GetEntityID()))
                HandleButton(items[i].GetEntityID());
        }
    }

    void UiSystem::HandleButton(EntityID eid)
    {
        if (!m_LeftClickThisFrame) return;

        auto* btn = m_Ctx.pScene->GetBehavior<UiButton>(eid);
        if (!btn || btn->IsDisabled) return;

        bool hit = false;
        if (m_Ctx.pScene->HasComponent<Raycastable>(eid))
        {
            const auto* ray = m_Ctx.pScene->GetComponent<Raycastable>(eid);
            if (!ray->Blocking) return;
            hit = TestRaycastable(*ray,
                                  { Inputs::GetMouseX(), Inputs::GetMouseY() },
                                  btn->pElement ? btn->pElement->GetPosition() : glm::vec2{ 0.0f, 0.0f },
                                  btn->pElement ? btn->pElement->GetScaledSize() : glm::vec2{ 0.0f, 0.0f });
        }
        else
        {
            hit = btn->IsMouseOver();
        }

        if (!hit) return;

        const int layer         = btn->pElement ? static_cast<int>(btn->pElement->Layer_) : 0;
        const int highestLayer  = mp_HighestButton ? static_cast<int>(mp_HighestButton->pElement->Layer_) : -2147000;

        if (layer > highestLayer)
        {
            mp_HighestButton = btn;
        }
        else if (layer == highestLayer && mp_HighestButton && btn->pElement)
        {
            if (btn->pElement->zIndex > mp_HighestButton->pElement->zIndex)
            {
                mp_HighestButton = btn;
            }
        }
    }

    void UiSystem::CallHighestButton()
    {
        if (m_LeftClickThisFrame && mp_HighestButton)
        {
            mp_PressedButton = mp_HighestButton;
            mp_PressedButton->IsClicked = true;
            mp_PressedButton->OnClick.Emit();
        }

        if (m_LeftReleaseThisFrame && mp_PressedButton)
        {
            mp_PressedButton->IsClicked = false;
            mp_PressedButton->OnRelease.Emit();
            mp_PressedButton = nullptr;
        }
    }

    void UiSystem::DrawChildren(const UiParentInformation& parent, Transform2D* child)
    {
        UiParentInformation next = parent;

        const EntityID eid = child->GetEntityID();

        if (m_Ctx.pScene->HasComponent<UiSprite>(eid))
        {
            auto* sprite = m_Ctx.pScene->GetComponent<UiSprite>(eid);
            if (!sprite->IsEnable) return;
            DrawSprite(sprite, child, parent);
            next = CreateBaseParentInformation(child, sprite);
        }
        else if (m_Ctx.pScene->HasComponent<UiText>(eid))
        {
            auto* text = m_Ctx.pScene->GetComponent<UiText>(eid);
            if (!text->IsEnable) return;
            DrawText(text, child, parent);
            next = CreateBaseParentInformation(child, text);
        }
        else if (m_Ctx.pScene->HasComponent<UiCanvas>(eid))
        {
            auto* canvas = m_Ctx.pScene->GetComponent<UiCanvas>(eid);
            if (!canvas->IsEnable) return;
            DrawCanvas(canvas, child, parent);
            next = CreateBaseParentInformation(child, canvas);
        }

        const uint32_t childCount = child->GetChildCount();
        std::vector<Transform2D*> sorted;
        sorted.reserve(childCount);
        for (uint32_t j = 0; j < childCount; ++j) sorted.push_back(child->GetChild(j));

        std::stable_sort(sorted.begin(), sorted.end(),
            [this](const Transform2D* a, const Transform2D* b)
            {
                const auto* ea = GetUiElement(a->GetEntityID());
                const auto* eb = GetUiElement(b->GetEntityID());
                const int32_t da = ea ? ea->DrawOrder : 0;
                const int32_t db = eb ? eb->DrawOrder : 0;
                return da < db;
            });

        for (auto* c : sorted) DrawChildren(next, c);
    }

    void UiSystem::Draw()
    {
        auto block = m_Ctx.pScene->GetComponentBlock<UiCanvas>();
        UiCanvas* canvases  = block->ComponentData();
        EntityID*  entities  = block->EntitiesData();
        const uint32_t count = block->GetEntityCount();

        m_DebugMaterial.Diffuse      = GPC_TEXTURE_ID("16x16_ui", "Ui_Black");
        m_DebugMaterial.TextureArray = GPC_TEXTURE_ARRAY("16x16_ui")->Array.GetUploadId();

        std::vector<uint32_t> order;
        order.reserve(count);
        for (uint32_t i = 0; i < count; ++i) order.push_back(i);
        std::stable_sort(order.begin(), order.end(),
            [canvases](uint32_t a, uint32_t b) { return canvases[a].DrawOrder < canvases[b].DrawOrder; });

        for (uint32_t i : order)
        {
            if (entities[i] == MAX_ENTITIES) continue;
            if (!canvases[i].IsEnable) continue;

            Transform3D* transform = m_Ctx.pScene->GetComponent<Transform3D>(entities[i]);
            if (!transform || transform->GetParent() != nullptr) continue;
            if (transform->GetChildCount() == 0 && !canvases[i].IsFitToScreen()) continue;

            m_CurrentSize = glm::vec2(m_Ctx.pWindow->GetSize());
            const glm::vec2 scale = m_CurrentSize / m_BaseSize;
            m_GlobalRatio = std::min(scale.x, scale.y);

            canvases[i].m_Size       = m_BaseSize;
            canvases[i].m_ScaledSize = m_CurrentSize;

            UiParentInformation info = CreateBaseParentInformation(transform, &canvases[i]);

            const uint32_t rootChildCount = transform->GetChildCount();
            std::vector<Transform2D*> sortedRoots;
            sortedRoots.reserve(rootChildCount);
            for (uint32_t j = 0; j < rootChildCount; ++j) sortedRoots.push_back(transform->GetChild(j));

            std::stable_sort(sortedRoots.begin(), sortedRoots.end(),
                [this](const Transform2D* a, const Transform2D* b)
                {
                    const auto* ea = GetUiElement(a->GetEntityID());
                    const auto* eb = GetUiElement(b->GetEntityID());
                    const int32_t da = ea ? ea->DrawOrder : 0;
                    const int32_t db = eb ? eb->DrawOrder : 0;
                    return da < db;
                });

            for (auto* c : sortedRoots) DrawChildren(info, c);
        }
    }

    UiElement* UiSystem::GetUiElement(EntityID eid) const
    {
        if (m_Ctx.pScene->HasComponent<UiSprite>(eid)) return m_Ctx.pScene->GetComponent<UiSprite>(eid);
        if (m_Ctx.pScene->HasComponent<UiText>(eid))   return m_Ctx.pScene->GetComponent<UiText>(eid);
        if (m_Ctx.pScene->HasComponent<UiCanvas>(eid)) return m_Ctx.pScene->GetComponent<UiCanvas>(eid);
        return nullptr;
    }

    bool UiSystem::IsAncestorDisabled(EntityID eid) const
    {
        auto* tr = m_Ctx.pScene->GetComponent<Transform2D>(eid);
        if (!tr) return false;

        Transform2D* parent = tr->GetParent();
        while (parent)
        {
            const UiElement* pe = GetUiElement(parent->GetEntityID());
            if (pe && !pe->IsEnable) return true;
            parent = parent->GetParent();
        }
        return false;
    }

    void UiSystem::DrawSprite(UiSprite* sprite, const Transform2D* transform, const UiParentInformation& parent)
    {
        const glm::vec2 position = glm::vec2(transform->LocalTransform.GetPosition()) * m_GlobalRatio
                                 + parent.Position
                                 + (sprite->Anchor_ * parent.Size)
                                 - sprite->Anchor_ * sprite->m_ScaledSize;

        m_RenderSpriteContainer.Position = position;
        m_RenderSpriteContainer.Size     = sprite->m_ScaledSize;
        m_RenderSpriteContainer.Tint     = sprite->Tint;
        m_RenderSpriteContainer.Rotation = sprite->Rotation;
        m_RenderSpriteContainer.Texture_  = sprite->m_Sprite;
        m_RenderSpriteContainer.Material_ = &sprite->m_Material;

        sprite->m_Position = position;
        sprite->Layer_      = parent.ParentLayer + 1;

        m_Ctx.pWindow->DrawObject(m_RenderSpriteContainer);
    }

    void UiSystem::DrawText(UiText* text, const Transform2D* transform, const UiParentInformation& parent)
    {
        const glm::vec2 base = glm::vec2(transform->LocalTransform.GetPosition()) * m_GlobalRatio
                             + parent.Position
                             + (text->Anchor_ * parent.Size);

        const glm::vec2 position = base - (text->Anchor_ * text->m_ScaledSize);

        m_RenderTextContainer.Position = position;
        m_RenderTextContainer.Scale    = text->m_FontSizeRatio * text->m_Scale.x * m_GlobalRatio;
        m_RenderTextContainer.Tint     = text->Tint;
        m_RenderTextContainer.Font_     = text->mp_Font;
        m_RenderTextContainer.Text     = text->m_Text;

        text->m_Position = position;
        text->Layer_      = parent.ParentLayer + 1;

        m_Ctx.pWindow->DrawText(m_RenderTextContainer);
    }

    void UiSystem::DrawCanvas(UiCanvas* canvas, const Transform2D* transform, const UiParentInformation& parent)
    {
        const glm::vec2 position = glm::vec2(transform->LocalTransform.GetPosition()) * m_GlobalRatio
                                 + parent.Position
                                 + (canvas->Anchor_ * parent.Size)
                                 - canvas->Anchor_ * canvas->m_ScaledSize;

        if (canvas->mp_Sprite)
        {
            m_RenderSpriteContainer.Position = position;
            m_RenderSpriteContainer.Size     = canvas->m_ScaledSize;
            m_RenderSpriteContainer.Tint     = canvas->Tint;
            m_RenderSpriteContainer.Rotation = canvas->Rotation;
            m_RenderSpriteContainer.Texture_  = canvas->mp_Sprite;
            m_RenderSpriteContainer.Material_ = &canvas->m_Material;
            m_Ctx.pWindow->DrawObject(m_RenderSpriteContainer);
        }
        else if (canvas->Tint.r() < 1.0f || glm::vec4(canvas->Tint).a < 1.0f)
        {
            m_RenderSpriteContainer.Position = position;
            m_RenderSpriteContainer.Size     = canvas->m_ScaledSize;
            m_RenderSpriteContainer.Tint     = canvas->Tint;
            m_RenderSpriteContainer.Rotation = canvas->Rotation;
            m_RenderSpriteContainer.Texture_  = &GPC_TEXTURE_ARRAY("16x16_ui")->Array;
            m_RenderSpriteContainer.Material_ = &m_DebugMaterial;
            m_Ctx.pWindow->DrawObject(m_RenderSpriteContainer);
        }

        canvas->m_Position = position;
        canvas->Layer_      = parent.ParentLayer + 1;
    }

    UiParentInformation UiSystem::CreateBaseParentInformation(Transform2D* transform, const UiElement* element) const
    {
        UiParentInformation info{};
        info.Position     = element->m_Position;
        info.Size         = element->m_ScaledSize;
        info.pTransform   = transform;
        info.ParentLayer  = element->Layer_;
        return info;
    }

    bool UiSystem::HasPressedButton() const
    {
        return mp_PressedButton != nullptr;
    }
}
