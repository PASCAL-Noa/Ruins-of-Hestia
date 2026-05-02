#include "UI/UiBuilder.h"

#include "Scene.h"
#include "TransformComponents.h"
#include "UI/UiLayout.h"
#include "UI/UiSystem.h"

namespace GPC
{
    UiBuilder::UiBuilder(Scene* pScene) : mp_Scene(pScene) {}

    UiBuilder& UiBuilder::At(Anchor anchor, glm::vec2 offset) { m_Anchor = anchor; m_Position = offset; return *this; }
    UiBuilder& UiBuilder::At(Anchor anchor) { m_Anchor = anchor; return *this; }
    UiBuilder& UiBuilder::Offset(glm::vec2 offset) { m_Position = offset; return *this; }
    UiBuilder& UiBuilder::Size(glm::vec2 size) { m_Size = size; return *this; }
    UiBuilder& UiBuilder::Scale(glm::vec2 scale) { m_Scale = scale; return *this; }
    UiBuilder& UiBuilder::Scale(float uniform) { m_Scale = { uniform, uniform }; return *this; }
    UiBuilder& UiBuilder::Tint(Color color) { m_Tint = color; return *this; }
    UiBuilder& UiBuilder::Bg(Color color) { m_BgColor = color; m_HasBgColor = true; return *this; }
    UiBuilder& UiBuilder::Bg(const std::string& atlas, const std::string& name) { m_BgAtlas = atlas; m_BgSprite = name; return *this; }
    UiBuilder& UiBuilder::Text(const std::string& content) { m_TextContent = content; return *this; }
    UiBuilder& UiBuilder::Font(const std::string& fontName) { m_FontName = fontName; return *this; }
    UiBuilder& UiBuilder::Font(const std::string& fontName, float size) { m_FontName = fontName; m_FontSize = size; return *this; }
    UiBuilder& UiBuilder::FontSize(float size) { m_FontSize = size; return *this; }
    UiBuilder& UiBuilder::Sprite(const std::string& atlas, const std::string& name) { m_Atlas = atlas; m_SpriteName = name; return *this; }

    UiBuilder& UiBuilder::ChildOf(EntityID parent) { m_Parent = parent; return *this; }

    UiBuilder& UiBuilder::Layout(UiLayoutMode mode) { m_LayoutMode = mode; m_HasLayout = true; return *this; }
    UiBuilder& UiBuilder::Gap(float gap) { m_Gap = gap; m_HasLayout = true; return *this; }
    UiBuilder& UiBuilder::Padding(glm::vec4 padding) { m_Padding = padding; m_HasLayout = true; return *this; }
    UiBuilder& UiBuilder::Padding(float uniform) { m_Padding = { uniform, uniform, uniform, uniform }; m_HasLayout = true; return *this; }
    UiBuilder& UiBuilder::FlexGrow(float grow) { m_FlexGrow = grow; return *this; }
    UiBuilder& UiBuilder::FitToScreen(bool fit) { m_FitToScreen = fit; return *this; }

    void UiBuilder::Reset()
    {
        m_Anchor      = Anchors::TOP_LEFT;
        m_Position    = { 0.0f, 0.0f };
        m_Size        = { 0.0f, 0.0f };
        m_Scale       = { 1.0f, 1.0f };
        m_Tint        = Colors::WHITE;
        m_Atlas       = "";
        m_SpriteName  = "";
        m_TextContent = "";
        m_FontName    = "";
        m_FontSize    = 0.0f;
        m_Parent      = MAX_ENTITIES;
        m_HasLayout   = false;
        m_LayoutMode  = UiLayoutMode::Stack;
        m_Gap         = 0.0f;
        m_Padding     = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_FlexGrow    = 0.0f;
        m_FitToScreen = false;
        m_BgColor     = Colors::TRANSPARENT;
        m_HasBgColor  = false;
        m_BgAtlas     = "";
        m_BgSprite    = "";
    }

    EntityID UiBuilder::CreateEntity()
    {
        const EntityID eid = mp_Scene->CreateEntityAs2D();

        auto* transform = mp_Scene->GetComponent<Transform2D>(eid);
        if (transform)
        {
            if (m_Parent != MAX_ENTITIES)
            {
                auto* parentTransform = mp_Scene->GetComponent<Transform2D>(m_Parent);
                if (parentTransform) transform->SetParent(parentTransform);

                if (mp_Scene->HasComponent<UiLayout>(m_Parent))
                {
                    mp_Scene->GetComponent<UiLayout>(m_Parent)->IsDirty = true;
                }
            }

            transform->LocalTransform.SetPosition({ m_Position.x, m_Position.y, 0.0f });
        }

        return eid;
    }

    void UiBuilder::SetupElement(UiElement* elt)
    {
        if (!elt) return;
        elt->Anchor_ = m_Anchor;
        elt->SetTint(m_Tint);
        elt->SetScale(m_Scale);
        if (m_Size.x > 0.0f && m_Size.y > 0.0f) elt->SetSize(m_Size);

        if (auto* sys = mp_Scene->GetSystem<UiSystem>())
        {
            elt->zIndex = sys->GetHighestzIndex() + 1;
            sys->IncrementZIndex();
        }
    }

    void UiBuilder::ApplyLayoutIfAny(EntityID eid)
    {
        if (!m_HasLayout && m_FlexGrow <= 0.0f) return;

        auto* layout = mp_Scene->AddComponent<UiLayout>(eid);
        if (!layout) return;

        if (m_HasLayout)
        {
            layout->Mode    = m_LayoutMode;
            layout->Gap     = m_Gap;
            layout->Padding = m_Padding;
        }
        layout->FlexGrow = m_FlexGrow;

        if (m_HasLayout && m_LayoutMode != UiLayoutMode::Stack)
        {
            if (auto* sys = mp_Scene->GetSystem<UiSystem>())
            {
                sys->RegisterLayoutRoot(eid);
            }
        }
    }

    UiCanvas* UiBuilder::BuildCanvas()
    {
        if (!mp_Scene) { Reset(); return nullptr; }

        const EntityID eid = CreateEntity();
        auto* canvas = mp_Scene->AddComponent<UiCanvas>(eid);
        if (!canvas) { Reset(); return nullptr; }

        if (m_HasBgColor)  canvas->SetBackgroundColor(m_BgColor);
        else               canvas->SetTint(m_Tint);

        if (!m_BgAtlas.empty() && !m_BgSprite.empty())
        {
            canvas->SetBackgroundSprite(m_BgAtlas, m_BgSprite);
        }

        SetupElement(canvas);
        canvas->SetFitToScreen(m_FitToScreen);

        ApplyLayoutIfAny(eid);
        Reset();
        return canvas;
    }

    UiText* UiBuilder::BuildText()
    {
        if (!mp_Scene) { Reset(); return nullptr; }

        const EntityID eid = CreateEntity();
        auto* text = mp_Scene->AddComponent<UiText>(eid);
        if (!text) { Reset(); return nullptr; }

        const std::string fontName = m_FontName.empty() ? std::string(UiTheme::DefaultFont) : m_FontName;
        text->SetFont(fontName);
        if (m_FontSize > 0.0f) text->SetFontSize(m_FontSize);
        text->SetText(m_TextContent);

        SetupElement(text);

        ApplyLayoutIfAny(eid);
        Reset();
        return text;
    }

    UiSprite* UiBuilder::BuildSprite()
    {
        if (!mp_Scene) { Reset(); return nullptr; }

        const EntityID eid = CreateEntity();
        auto* sprite = mp_Scene->AddComponent<UiSprite>(eid);
        if (!sprite) { Reset(); return nullptr; }

        if (!m_Atlas.empty() && !m_SpriteName.empty())
        {
            sprite->SetSprite(m_Atlas, m_SpriteName);
        }

        SetupElement(sprite);

        ApplyLayoutIfAny(eid);
        Reset();
        return sprite;
    }

    UiButton* UiBuilder::BuildButton(UiElement* target)
    {
        if (!mp_Scene || !target || target->GetEntityID() == MAX_ENTITIES) return nullptr;
        return mp_Scene->AddBehavior<UiButton>(target->GetEntityID());
    }
}
