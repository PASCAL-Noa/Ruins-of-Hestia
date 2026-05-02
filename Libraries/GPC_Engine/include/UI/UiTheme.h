#pragma once

#include "Color.h"
#include <glm/glm.hpp>

namespace GPC::UiTheme
{
    inline const Color PanelBackground     = Color(51, 51, 56, 230);
    inline const Color PanelPrimary        = Color(60, 90, 150, 240);
    inline const Color PanelWarning        = Color(170, 120, 50, 240);
    inline const Color PanelDanger         = Color(180, 70, 70, 240);

    inline const Color TitleText           = Color(243, 243, 235, 255);
    inline const Color BodyText            = Color(220, 220, 215, 255);
    inline const Color DimmedText          = Color(170, 170, 160, 255);
    inline const Color DarkText            = Colors::PERFECT_BROWN;
    inline const Color WhiteText            = Colors::WHITE;

    inline const Color ButtonPrimary       = Color(90, 128, 205, 255);
    inline const Color ButtonPrimaryHover  = Color(112, 148, 222, 255);
    inline const Color ButtonPrimaryDown   = Color(70, 108, 180, 255);
    inline const Color ButtonDisabled      = Color(70, 70, 75, 200);

    inline const Color TooltipBackground   = Color(30, 30, 35, 240);
    inline const Color TooltipBorder       = Color(90, 90, 100, 255);

    inline const Color AccentGold          = Color(215, 168, 60, 255);
    inline const Color AccentCyan          = Color(80, 180, 200, 255);

    inline constexpr float PanelPaddingSmall   = 8.0f;
    inline constexpr float PanelPaddingMedium  = 16.0f;
    inline constexpr float PanelPaddingLarge   = 24.0f;

    inline constexpr float PanelMargin         = 16.0f;
    inline constexpr float ElementMargin       = 8.0f;
    inline constexpr float RowGap              = 12.0f;
    inline constexpr float ColumnGap           = 12.0f;

    inline constexpr float TitleFontSize       = 24.0f;
    inline constexpr float SubtitleFontSize    = 20.0f;
    inline constexpr float BodyFontSize        = 16.0f;
    inline constexpr float SmallFontSize       = 12.0f;

    inline constexpr float ButtonHeight        = 42.0f;
    inline constexpr float IconSmall           = 24.0f;
    inline constexpr float IconMedium          = 48.0f;
    inline constexpr float IconLarge           = 64.0f;

    inline constexpr const char* DefaultFont   = "Eczar";
}
