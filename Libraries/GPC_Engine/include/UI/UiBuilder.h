#pragma once

#include <string>

#include "ECS_Defines.h"
#include "UI/UiButton.h"
#include "UI/UiCanvas.h"
#include "UI/UiElement.h"
#include "UI/UiLayout.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"
#include "UI/UiTheme.h"

namespace GPC
{
    class Scene;

    class UiBuilder
    {
    public:
        explicit UiBuilder(Scene* pScene);

        UiBuilder& At(Anchor anchor, glm::vec2 offset);
        UiBuilder& At(Anchor anchor);
        UiBuilder& Offset(glm::vec2 offset);
        UiBuilder& Size(glm::vec2 size);
        UiBuilder& Scale(glm::vec2 scale);
        UiBuilder& Scale(float uniform);
        UiBuilder& Tint(Color color);
        UiBuilder& Bg(Color color);
        UiBuilder& Bg(const std::string& atlas, const std::string& name);
        UiBuilder& Text(const std::string& content);
        UiBuilder& Font(const std::string& fontName);
        UiBuilder& Font(const std::string& fontName, float size);
        UiBuilder& FontSize(float size);
        UiBuilder& Sprite(const std::string& atlas, const std::string& name);
        UiBuilder& ChildOf(EntityID parent);

        UiBuilder& Layout(UiLayoutMode mode);
        UiBuilder& Gap(float gap);
        UiBuilder& Padding(glm::vec4 padding);
        UiBuilder& Padding(float uniform);
        UiBuilder& FlexGrow(float grow);
        UiBuilder& FitToScreen(bool fit);

        UiCanvas* BuildCanvas();
        UiText*   BuildText();
        UiSprite* BuildSprite();
        UiButton* BuildButton(UiElement* target);

    private:
        void     Reset();
        EntityID CreateEntity();
        void     SetupElement(UiElement* elt);
        void     ApplyLayoutIfAny(EntityID eid);

    private:
        Scene*      mp_Scene      = nullptr;

        Anchor      m_Anchor      = Anchors::TOP_LEFT;
        glm::vec2   m_Position    = { 0.0f, 0.0f };
        glm::vec2   m_Size        = { 0.0f, 0.0f };
        glm::vec2   m_Scale       = { 1.0f, 1.0f };
        Color       m_Tint        = Colors::WHITE;
        std::string m_Atlas       = "";
        std::string m_SpriteName  = "";
        std::string m_TextContent = "";
        std::string m_FontName    = "";
        float       m_FontSize    = 0.0f;
        EntityID    m_Parent      = MAX_ENTITIES;

        bool          m_HasLayout   = false;
        UiLayoutMode m_LayoutMode  = UiLayoutMode::Stack;
        float         m_Gap         = 0.0f;
        glm::vec4     m_Padding     = { 0.0f, 0.0f, 0.0f, 0.0f };
        float         m_FlexGrow    = 0.0f;
        bool          m_FitToScreen = false;
        Color         m_BgColor     = Colors::TRANSPARENT;
        bool          m_HasBgColor  = false;
        std::string   m_BgAtlas     = "";
        std::string   m_BgSprite    = "";
    };
}
