#pragma once

#include <string>

#include "Material.h"
#include "UI/UiElement.h"

namespace GPC
{
    struct Texture;

    struct UiCanvas : public UiElement
    {
        BIND_COMPONENT(UI_CANVAS);
        AUTO_UI_COMPONENT_CONSTRUCTOR(UiCanvas);

        virtual ~UiCanvas() = default;

        void SetSize(const glm::vec2& size) override;
        void CalcElementSize() override {}

        void SetBackgroundColor(const Color& color) { SetTint(color); }
        void SetBackgroundSprite(const std::string& atlas, const std::string& name);

        void SetFitToScreen(bool fit) { m_FitToScreen = fit; }
        [[nodiscard]] bool IsFitToScreen() const { return m_FitToScreen; }

    private:
        const Texture*  mp_Sprite     = nullptr;
        Material        m_Material    = {};
        bool            m_FitToScreen = false;

        friend class UiSystem;
    };
}
