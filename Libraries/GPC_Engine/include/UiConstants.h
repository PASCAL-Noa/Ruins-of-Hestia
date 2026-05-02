#pragma once

#include <glm/glm.hpp>

namespace GPC
{
    inline constexpr float kBaseResolutionWidth  = 1080.0f;
    inline constexpr float kBaseResolutionHeight = 720.0f;

    inline constexpr float kRaycastInvalidSentinel = 1.0e10f;

    inline constexpr int   kDialogueMaxLinesPerPage   = 4;
    inline constexpr float kDialogueBeginPosX         = 190.0f;
    inline constexpr float kDialogueMarginX           = 20.0f;
    inline constexpr float kDialogueMarginY           = 20.0f;
    inline constexpr float kDialogueLineStartY        = 50.0f;
    inline constexpr float kDialogueLineOffset        = 35.0f;
    inline constexpr float kDialogueBoxAtlasWidth     = 1411.0f;

    inline constexpr float kHudScale            = 0.8f;
    inline constexpr float kHudElementMargin    = 5.0f;
    inline constexpr float kHpTextFontSize      = 15.0f;

    inline constexpr glm::vec2 kVillagePanelSize  = { 400.0f, 500.0f };
    inline constexpr glm::vec2 kBuildingPanelSize = { 471.0f, 516.0f };
    inline constexpr glm::vec2 kTooltipPanelSize  = { 786.0f, 860.0f };

    inline constexpr float kBuildingRowStartY   = 70.0f;
    inline constexpr float kBuildingRowHeight   = 72.0f;

    inline constexpr uint32_t kCountResourcesBasic = 5;

#ifdef NDEBUG
    inline constexpr bool kDebugOverlayEnabled = false;
#else
    inline constexpr bool kDebugOverlayEnabled = true;
#endif
}
