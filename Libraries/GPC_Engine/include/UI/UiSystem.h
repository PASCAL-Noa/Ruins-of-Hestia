#ifndef GPC_UI_SYSTEM_H
#define GPC_UI_SYSTEM_H

#include <vector>

#include "ECS_Defines.h"
#include "Material.h"
#include "RenderObject2D.h"
#include "System.h"
#include "UI/UiElement.h"

namespace GPC
{
    class Scene;
    struct UiButton;
    struct UiCanvas;
    struct UiSprite;
    struct UiText;

    void ResolveLayoutFor(Scene& scene, EntityID rootId);

    class UiSystem : public System
    {
    public:
        BIND_SYSTEM(UI);

        void OnUpdate() override;
        void OnLateUpdate() override;

        void  RegisterLayoutRoot(EntityID eid);
        void  UnregisterLayoutRoot(EntityID eid);

        void  OnEntityDestroyed(EntityID eid);

        int   GetHighestzIndex() const { return m_HighestzIndex; }
        void  IncrementZIndex() { ++m_HighestzIndex; }
        float GetGlobalRatio() const { return m_GlobalRatio; }

        bool HasPressedButton() const;

        void EnableUI() { m_GlobalEnable = true; }
        void DisableUI() { m_GlobalEnable = false; }

    private:
        void UpdateTexts();
        void UpdateSprites();
        void UpdateCanvases();
        void UpdateAnims();

        void HandleButton(EntityID eid);
        void CallHighestButton();

        void Draw();
        void DrawChildren(const UiParentInformation& parent, Transform2D* child);

        void DrawSprite(UiSprite* sprite, const Transform2D* transform, const UiParentInformation& parent);
        void DrawText(UiText* text, const Transform2D* transform, const UiParentInformation& parent);
        void DrawCanvas(UiCanvas* canvas, const Transform2D* transform, const UiParentInformation& parent);

        UiParentInformation CreateBaseParentInformation(Transform2D* transform, const UiElement* element) const;

        UiElement* GetUiElement(EntityID eid) const;

        bool IsAncestorDisabled(EntityID eid) const;

    private:
        RenderObject2D  m_RenderSpriteContainer{};
        RenderText2D    m_RenderTextContainer{};
        Material        m_DebugMaterial{};

        glm::vec2       m_BaseSize             = { 1080.0f, 720.0f };
        glm::vec2       m_CurrentSize          = { 1080.0f, 720.0f };
        float           m_GlobalRatio          = 0.0f;
        int             m_HighestzIndex        = 0;

        UiButton*      mp_HighestButton       = nullptr;
        UiButton*      mp_PressedButton       = nullptr;
        bool            m_LeftClickThisFrame   = false;
        bool            m_LeftReleaseThisFrame = false;

        std::vector<EntityID> m_LayoutRoots;

        bool            m_GlobalEnable = true;
    };
}

#endif // GPC_UI_SYSTEM_H
