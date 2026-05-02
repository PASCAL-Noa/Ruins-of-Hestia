#pragma once

#include <glm/glm.hpp>

#include "Color.h"
#include "Component.h"
#include "TransformComponents.h"

#define AUTO_UI_COMPONENT_CONSTRUCTOR(type_t) type_t(EntityID eid) : UiElement(eid) {}; type_t() : UiElement() {};

namespace GPC
{
    class Scene;
    class UiSystem;
    struct UiElement;

    using Anchor = glm::vec2;
    using Layer  = uint32_t;

    namespace Anchors
    {
        constexpr Anchor TOP_LEFT       = { 0.0f, 0.0f };
        constexpr Anchor TOP_MIDDLE     = { 0.5f, 0.0f };
        constexpr Anchor TOP_RIGHT      = { 1.0f, 0.0f };
        constexpr Anchor MIDDLE_LEFT    = { 0.0f, 0.5f };
        constexpr Anchor MIDDLE_MIDDLE  = { 0.5f, 0.5f };
        constexpr Anchor MIDDLE_RIGHT   = { 1.0f, 0.5f };
        constexpr Anchor BOTTOM_LEFT    = { 0.0f, 1.0f };
        constexpr Anchor BOTTOM_MIDDLE  = { 0.5f, 1.0f };
        constexpr Anchor BOTTOM_RIGHT   = { 1.0f, 1.0f };
    }

    struct UiParentInformation
    {
        Transform3D*    pTransform      = nullptr;
        glm::vec2       Position        = { 0.0f, 0.0f };
        glm::vec2       Size            = { 0.0f, 0.0f };
        Layer           ParentLayer     = 0;
    };

    struct UiElement : public Component
    {
        static constexpr bool kStableSlot = true;

        Anchor          Anchor_          = Anchors::TOP_LEFT;
        Layer           Layer_           = 0;
        uint32_t        zIndex          = 0;
        int32_t         DrawOrder       = 0;
        float           Rotation        = 0.0f;
        bool            IsButton        = false;

        UiElement() = default;
        UiElement(EntityID eid) : Component(eid) {}
        virtual ~UiElement() = default;

        void SetTint(const Color& tint) { Tint = tint; }
        void SetScale(const glm::vec2& scale) { m_Scale = scale; }

        virtual void SetSize(const glm::vec2& size) { m_Size = size; }
        virtual void SetPosition(const glm::vec2& position) { m_Position = position; }
        virtual void CalcElementSize() = 0;

        [[nodiscard]] glm::vec2 GetSize() const       { return m_Size; }
        [[nodiscard]] glm::vec2 GetScale() const      { return m_Scale; }
        [[nodiscard]] glm::vec2 GetScaledSize() const { return m_ScaledSize; }
        [[nodiscard]] glm::vec2 GetPosition() const   { return m_Position; }
        [[nodiscard]] Color     GetTint() const       { return Tint; }

    protected:
        glm::vec2   m_Size       = { 0.0f, 0.0f };
        glm::vec2   m_Scale      = { 1.0f, 1.0f };
        glm::vec2   m_ScaledSize = { 0.0f, 0.0f };
        glm::vec2   m_Position   = { 0.0f, 0.0f };
        Color       Tint         = Colors::WHITE;

        friend class UiSystem;
    };
}
