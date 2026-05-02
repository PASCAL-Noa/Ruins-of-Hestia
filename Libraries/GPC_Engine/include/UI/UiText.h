#pragma once

#include <string>
#include <string_view>

#include "UI/UiElement.h"
#include "UI/UiReactive.h"
#include "UI/UiSignal.h"

namespace GPC
{
    struct Font;

    struct UiText : public UiElement
    {
        BIND_COMPONENT(UI_TEXT);
        AUTO_UI_COMPONENT_CONSTRUCTOR(UiText);

        virtual ~UiText() = default;

        void CalcElementSize() override;

        void SetText(std::string_view text);
        void SetFontSize(float size);
        void SetFont(const Font* pFont);
        void SetFont(const std::string& fontName);

        [[nodiscard]] const std::string& GetText() const { return m_Text; }

        void BindReactive(UiReactive<std::string>& source);
        void BindReactive(UiReactive<int>& source);
        void BindReactive(UiReactive<float>& source);

    private:
        std::string      m_Text          = "";
        bool             m_Updated       = false;
        float            m_FontSize      = 16.0f;
        float            m_FontSizeRatio = 1.0f;
        const Font*      mp_Font         = nullptr;

        UiSignalHandle             m_BoundHandle        = 0;
        UiReactive<std::string>*   mp_BoundStringSource = nullptr;
        UiReactive<int>*           mp_BoundIntSource    = nullptr;
        UiReactive<float>*         mp_BoundFloatSource  = nullptr;

        char m_FormatBuffer[32] = {};

        friend class UiSystem;
    };
}
